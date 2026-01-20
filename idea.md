For the spec overlay, that’s the kind of thing I’d prefer to embed in dicl. I want dicl to be the single place where differences between a given FC40 srpm and a RSE srpm are stored. And that may be the true goal: dicl or something like it as a conversion utility. Give it a FC40 srpm and it applies a catalog of generic and package specific fixes to convert that srpm for irix

For the spec overlay, that’s the kind of thing I’d prefer to embed in dicl. I want dicl to be the single place where differences between a given FC40 srpm and a RSE srpm are stored. And that may be the true goal: dicl or something like it as a conversion utility. Give it a FC40 srpm and it applies a catalog of generic and package specific fixes to convert that srpm for irix

Yes — that is the clean architectural center.
What you just described is the missing unifying idea that explains why libdicl existed, why RPM worked, and why Fedora was tolerable.

Let me restate it crisply, then propose a concrete shape for it that stays true to SGUG history and avoids new complexity.

The real goal (now clearly articulated)

dicl is not a compatibility library.
dicl is a semantic adapter between Linux SRPM intent and IRIX reality.

Everything else follows from that.

Not:

a distro

a ports tree

a replacement RPM system

But:

A deterministic SRPM → RSE-SRPM conversion engine

That is a much stronger and more defensible goal.

Why dicl is the right place for this (not spec overlays)

You could bolt this logic onto spec files, but that would be the wrong direction:

Spec overlays duplicate logic per package

Fedora churn forces repeated edits

Knowledge fragments across hundreds of specs

Review becomes impossible

By contrast, embedding this in dicl gives you:

A single choke point for platform knowledge

Reuse across all packages

Auditable, testable behavior

A tool others can run without understanding IRIX

This matches how libdicl already behaved historically — it just wasn’t named as such.

Reframing dicl’s role (important shift)
Old implicit role

“Make Linux-ish code compile on IRIX”

New explicit role

“Transform a Linux SRPM into an IRIX-consumable SRPM + build plan”

That transformation includes:

source patches

spec rewriting

macro injection

dependency reinterpretation

toolchain normalization

This is compiler frontend thinking, not packaging thinking.

A clean mental model: dicl as an SRPM compiler

Think of it like this:

FC40 SRPM
   |
   |  (parse spec + sources)
   v
dicl
   |
   |  (apply rules, rewrite intent)
   v
RSE SRPM


Where dicl contains all platform knowledge.

This mirrors:

clang frontends

autoconf → configure

meson → ninja

IRIX MIPSpro → GCC migrations

You’re not building packages.
You’re compiling build intent.

What dicl should own (explicitly)
1. Generic Linux → IRIX transformations

These are not package-specific:

Remove / ignore:

systemd, selinux, udev

kernel headers

glibc-langpack

Rewrite paths:

/usr/lib → /usr/sgug/lib32

/usr/include → /usr/sgug/include

Normalize toolchain:

gcc → irix-clang

ld → irix-ld

Enforce ABI:

N32 only

-D_LANGUAGE_C

_LONGLONG

Disable features globally:

pthread assumptions

epoll

inotify

__thread

These belong in generic dicl rules, not specs.

2. Package-class rules (families, not one-offs)

Examples:

Compression libs

static-only builds allowed

no runtime deps

Autotools projects

forced ac_cv_* cache

skip run tests

GNU gnulib-heavy projects

prefer dicl replacements

disable locale probing

This is where dicl shines over pkgsrc:
you encode patterns, not patches.

3. Package-specific exceptions (small, explicit)

These are unavoidable, but contained:

packages:
  zlib:
    drop_requires:
      - zlib-devel
    force_static: true

  tree:
    disable_features:
      - gettext
    patch:
      - tree-irix-fix.patch


Critically:

These live outside the spec

They survive Fedora upgrades

They are readable by non-IRIX people

What dicl should not do

This is equally important.

dicl should not:

run rpmbuild itself

install packages

manage repos

replace RPM metadata

solve dependencies dynamically

Those are downstream concerns.

dicl produces input for existing tooling.

How this avoids past failures
Nekoware

Too manual

Knowledge trapped in people’s heads

irixports / pkgsrc

Knowledge duplicated per package

High per-port cost

Pure RPM

Platform knowledge scattered

Spec drift over time

dicl-as-converter

Centralized knowledge

Fedora remains upstream

IRIX remains downstream

Tooling remains standard

This is the first model that scales and is honest about IRIX.

A very concrete dicl interface (sketch)

Something like:

dicl convert \
  --srpm zlib-1.2.13-5.fc40.src.rpm \
  --target irix6.5-n32 \
  --output ./zlib-rse.srpm


Internally:

Extract SRPM

Parse spec

Apply rule engine

Patch sources

Rewrite spec

Emit new SRPM + report

The report matters — it’s how you debug.

Why this is better than spec overlays
Aspect	Spec overlays	dicl
Knowledge location	Fragmented	Centralized
Fedora upgrades	Painful	Mostly automatic
Reviewability	Poor	Excellent
Onboarding	Hard	Easy
Long-term survival	Low	High

This is exactly the lesson SGUG learned the hard way.

The subtle but crucial win

Once dicl exists as a conversion utility, several things become true:

You can swap Fedora versions freely (31 → 40 → 45)

You can experiment with non-RPM outputs later

You can test conversions without touching IRIX

You can invite contributors who don’t own SGI hardware

That last point matters a lot in 2026.

Bottom line

You’ve landed on the correct end-state:

dicl is the institutional memory of how to turn Linux intent into IRIX reality.

libdicl (the library) was just the first artifact of that idea.
The real product is the knowledge compiler.

If you want, next we can:

define dicl’s rule DSL (YAML vs code),

design its internal pipeline,

or take one FC40 SRPM and walk through what dicl would do step by step.

This is a strong, coherent direction.
