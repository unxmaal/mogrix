# WebKit Memory Optimization for IRIX

## Problem

WebKit defaults are designed for modern desktops with 8-32GB RAM. On IRIX machines
(256MB-1.5GB RAM, 2GB n32 virtual address limit), heavy JS sites like Google.com
exhaust memory and kill the browser.

## Research: Constrained WebKit ports

Best practices from other resource-constrained WebKit ports:

- **PlayStation port** (upstream): all JIT OFF, IsoMalloc OFF, GPU process OFF,
  periodic memory monitor ON
- **WPE WebKit** (official embedded port): JSC GC tuning env vars (`JSC_forceRAMSize`,
  growth factors, `criticalGCMemoryThreshold=0.5`), memory pressure API
- **Haiku**: WebKitLegacy (single-process), system malloc, all GPU/media disabled
- **AmigaOS/Odyssey**: single-threaded, no JIT, static linking

## C code changes (main.c)

### Memory pressure settings (biggest impact)

```c
WebKitMemoryPressureSettings *memPressure = webkit_memory_pressure_settings_new();
webkit_memory_pressure_settings_set_memory_limit(memPressure, 512);
webkit_memory_pressure_settings_set_conservative_threshold(memPressure, 0.33);
webkit_memory_pressure_settings_set_strict_threshold(memPressure, 0.50);
webkit_memory_pressure_settings_set_kill_threshold(memPressure, 0.90);
webkit_memory_pressure_settings_set_poll_interval(memPressure, 10);
webkit_website_data_manager_set_memory_pressure_settings(memPressure);
webkit_memory_pressure_settings_free(memPressure);
```

Thresholds match WPE defaults (0.33 conservative, 0.50 strict). 512MB limit per
WebProcess. Poll every 10 seconds. Call this in `activate()` after creating the
WebKitWebsiteDataManager.

### Cache model

```c
webkit_web_context_set_cache_model(webContext, WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);
```

Disables memory and disk cache. Saves 30-50MB. PlayStation port uses the same approach.

### Disable process swap

```c
"process-swap-on-cross-site-navigation-enabled", FALSE,
```

Each cross-origin navigation spawns a new WebProcess (~80-120MB). Disabling keeps one
WebProcess alive, dramatically reducing memory for multi-site browsing.

### Disable memory-hungry features

```c
webkit_settings_set_enable_page_cache(webkitSettings, FALSE);
webkit_settings_set_enable_offline_web_application_cache(webkitSettings, FALSE);
webkit_settings_set_enable_html5_local_storage(webkitSettings, FALSE);
webkit_settings_set_enable_html5_database(webkitSettings, FALSE);
webkit_settings_set_media_playback_requires_user_gesture(webkitSettings, TRUE);
webkit_settings_set_enable_media(webkitSettings, FALSE);
webkit_settings_set_enable_webaudio(webkitSettings, FALSE);
```

Each saves 5-20MB. Mirrors PlayStation/Haiku disabled feature sets.

## Launcher env vars (bundle.py)

JSC garbage collector tuning (WPE embedded pattern):

```bash
# Tell JSC the machine has limited RAM (512MB)
: ${JSC_forceRAMSize=536870912}
export JSC_forceRAMSize

# Aggressive GC: trigger at 50% heap usage (vs 80% default)
: ${JSC_criticalGCMemoryThreshold=0.50}
export JSC_criticalGCMemoryThreshold

# Slow heap growth (1.1x instead of default ~2x)
: ${JSC_smallHeapGrowthFactor=1.1}
export JSC_smallHeapGrowthFactor
: ${JSC_mediumHeapGrowthFactor=1.1}
export JSC_mediumHeapGrowthFactor
: ${JSC_largeHeapGrowthFactor=1.1}
export JSC_largeHeapGrowthFactor
```

The `: ${VAR=default}` syntax allows user override. These are injected by
`mogrix/bundle.py` into WebKit bundle launcher scripts.

## Test results

| Site | Before | After |
|------|--------|-------|
| example.com | Works | Works |
| news.ycombinator.com | Works | Works |
| kagi.com | Works | Works |
| google.com | OOM kills entire browser | WebProcess crashes, browser survives |

Google.com's JS is simply too heavy for 512MB. The memory pressure system converts a
fatal OOM (whole browser killed) into a graceful WebProcess crash (browser stays alive,
user can navigate elsewhere). This is the expected behavior for embedded WebKit ports.

## Files

- `patches/packages/ir8/main.c` — all C code changes
- `mogrix/bundle.py` — JSC GC env vars (line ~1257)
- `rules/packages/ir8.yaml` — package rules
