// pmr_shim.cc — Provides std::pmr symbols missing from clang-built libstdc++.
//
// GCC 9's memory_resource.cc fails to compile with clang 16 due to:
//   1. Out-of-line "= default" virtual destructor (clang rejects)
//   2. 3-arg operator delete (sized dealloc behind __cpp_sized_deallocation)
//   3. Inline destructor redefinition (header vs source mismatch)
//
// This shim extracts only the 3 symbols Qt5 needs:
//   - std::pmr::get_default_resource()
//   - std::pmr::monotonic_buffer_resource::_M_new_buffer(size_t, size_t)
//   - std::pmr::monotonic_buffer_resource::_M_release_buffers()
//
// Built with: -std=gnu++17 -D_GLIBCXX_USE_ALIGNED_ALLOC=1
//
#include <atomic>
#include <memory_resource>
#include <algorithm>  // std::max
#include <new>
#include <cstddef>
#include <cstring>    // __builtin_memcpy
#include <bit>        // __log2p1, __ceil2 (GCC 9 internal)

namespace std _GLIBCXX_VISIBILITY(default) {
_GLIBCXX_BEGIN_NAMESPACE_VERSION
namespace pmr {

  // Base class virtual destructor — GCC's memory_resource.cc defines this
  // out-of-line as "= default" which older clang rejected. Provide explicitly.
  memory_resource::~memory_resource() {}

  // --- get_default_resource ---
  // Uses the same atomic from the header's set_default_resource.
  // The default_resource atomic is defined in the memory_resource header
  // as an inline variable (C++17). get_default_resource just loads it.
  //
  // Actually, in GCC 9, the atomic and the new_delete_resource are
  // defined in the .cc file, not as inline variables. We need to provide
  // the full default resource infrastructure.

  namespace {
    class newdel_res_t final : public memory_resource {
      void* do_allocate(size_t bytes, size_t alignment) override {
        // Use aligned allocation for over-aligned requests
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
          return ::operator new(bytes, std::align_val_t(alignment));
        return ::operator new(bytes);
      }
      void do_deallocate(void* p, size_t bytes, size_t alignment) noexcept override {
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
          ::operator delete(p, std::align_val_t(alignment));
        else
          ::operator delete(p);
      }
      bool do_is_equal(const memory_resource& other) const noexcept override {
        return &other == this;
      }
    };

    // Aligned storage to avoid static constructor
    alignas(newdel_res_t) unsigned char newdel_buf[sizeof(newdel_res_t)];
    alignas(newdel_res_t) unsigned char null_buf[sizeof(newdel_res_t)];

    // Lazily construct on first use
    newdel_res_t* newdel_res() {
      static newdel_res_t* p = ::new(newdel_buf) newdel_res_t;
      return p;
    }

    // The global default resource pointer
    std::atomic<memory_resource*>& default_res() {
      static std::atomic<memory_resource*> r{nullptr};
      return r;
    }
  }

  memory_resource* new_delete_resource() noexcept {
    return newdel_res();
  }

  memory_resource* null_memory_resource() noexcept {
    // null resource always throws bad_alloc
    class null_res_t final : public memory_resource {
      void* do_allocate(size_t, size_t) override {
        throw std::bad_alloc();
      }
      void do_deallocate(void*, size_t, size_t) noexcept override { }
      bool do_is_equal(const memory_resource& other) const noexcept override {
        return &other == this;
      }
    };
    static null_res_t* p = ::new(null_buf) null_res_t;
    return p;
  }

  memory_resource* set_default_resource(memory_resource* r) noexcept {
    if (r == nullptr)
      r = new_delete_resource();
    return default_res().exchange(r);
  }

  memory_resource* get_default_resource() noexcept {
    memory_resource* r = default_res().load();
    if (r == nullptr)
      r = new_delete_resource();
    return r;
  }

  // --- monotonic_buffer_resource internals ---

  // _Chunk: bookkeeping struct placed at end of each allocated block.
  // Stores size/alignment as log2 and a pointer to the previous chunk.
  class monotonic_buffer_resource::_Chunk {
  public:
    static std::pair<void*, size_t>
    allocate(memory_resource* r, size_t size, size_t align, _Chunk*& head) {
      // Round up to power of 2, plus space for _Chunk at the end
      size_t alloc_size = size + sizeof(_Chunk);
      // __ceil2: round up to next power of 2
      alloc_size = std::__ceil2(alloc_size);

      if (align < alignof(_Chunk))
        align = alignof(_Chunk);

      void* p = r->allocate(alloc_size, align);
      void* back = static_cast<char*>(p) + alloc_size - sizeof(_Chunk);
      head = ::new(back) _Chunk(alloc_size, align, head);
      return {p, alloc_size - sizeof(_Chunk)};
    }

    static void
    release(_Chunk*& head, memory_resource* r) noexcept {
      _Chunk* next = head;
      head = nullptr;
      while (next) {
        _Chunk* ch = next;
        __builtin_memcpy(&next, ch->_M_next, sizeof(_Chunk*));

        if (ch->_M_canary != (ch->_M_size | ch->_M_align))
          return; // buffer overflow detected

        size_t size = static_cast<size_t>(1) << ch->_M_size;
        size_t align = static_cast<size_t>(1) << ch->_M_align;
        void* start = reinterpret_cast<char*>(ch + 1) - size;
        r->deallocate(start, size, align);
      }
    }

  private:
    _Chunk(size_t size, size_t align, _Chunk* next) noexcept
    : _M_size(static_cast<unsigned char>(std::__log2p1(size) - 1)),
      _M_align(static_cast<unsigned char>(std::__log2p1(align) - 1))
    {
      __builtin_memcpy(_M_next, &next, sizeof(next));
      _M_canary = _M_size | _M_align;
    }

    unsigned char _M_canary;
    unsigned char _M_size;
    unsigned char _M_align;
    unsigned char _M_next[sizeof(_Chunk*)];
  };

  void
  monotonic_buffer_resource::_M_new_buffer(size_t bytes, size_t alignment) {
    const size_t n = std::max(bytes, _M_next_bufsiz);
    const size_t m = std::max(alignment, alignof(std::max_align_t));
    auto [p, size] = _Chunk::allocate(_M_upstream, n, m, _M_head);
    _M_current_buf = p;
    _M_avail = size;
    _M_next_bufsiz *= _S_growth_factor;
  }

  void
  monotonic_buffer_resource::_M_release_buffers() noexcept {
    _Chunk::release(_M_head, _M_upstream);
  }

} // namespace pmr
_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std
