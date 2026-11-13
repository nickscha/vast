#ifndef VAST_PROFILER_H
#define VAST_PROFILER_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h" /* Basic types (u8, u32, ...)    */

/* #############################################################################
 * # [SECTION] Profiler Structs
 * #############################################################################
 */
typedef struct vast_profiler_entry
{
    f64 time_ms_last;
    f64 time_ms_min;
    f64 time_ms_max;
    f64 time_ms_total;
    u32 line;
    u32 counter;
    s8 *name;
    s8 *file;

} vast_profiler_entry;

/* #############################################################################
 * # [SECTION] Profiler Release Version
 * #############################################################################
 */
#ifdef VAST_RELEASE

/* On Release build the profiler is disabled */
#define VAST_PROFILER_BEGIN(name)
#define VAST_PROFILER_END(name)
#define VAST_PROFILER_COUNT(name)

static vast_profiler_entry vast_profiler_entries[1];
static u32 vast_profiler_entries_count = 0;

#else

/* #############################################################################
 * # [SECTION] Performance Profiler
 * #############################################################################
 */
#ifndef VAST_PROFILER_MAX_ENTRIES
#define VAST_PROFILER_MAX_ENTRIES 512
#endif

#define VAST_PROFILER_ENTRY_INVALID 0xFFFFFFFF

static vast_profiler_entry vast_profiler_entries[VAST_PROFILER_MAX_ENTRIES];
static u32 vast_profiler_entries_count = 0;

VAST_API f64 vast_profiler_time_ms(void);

VAST_API VAST_INLINE u32 vast_profiler_string_equals(s8 *a, s8 *b)
{
    while (*a && *b)
    {
        if (*a != *b)
        {
            return 0;
        }

        a++;
        b++;
    }

    return (*a == *b);
}

VAST_API VAST_INLINE u32 vast_profiler_find_entry(s8 *name)
{
    u32 i;

    for (i = 0; i < vast_profiler_entries_count; ++i)
    {
        if (vast_profiler_string_equals(vast_profiler_entries[i].name, name))
        {
            return i;
        }
    }

    return VAST_PROFILER_ENTRY_INVALID;
}

VAST_API VAST_INLINE void vast_profiler_begin(s8 *name, s8 *file, u32 line)
{
    u32 entry_id = vast_profiler_find_entry(name);

    if (entry_id == VAST_PROFILER_ENTRY_INVALID)
    {
        vast_profiler_entry entry = {0};
        entry.name = name;
        entry.file = file;
        entry.line = line;
        entry.counter += 1;
        entry.time_ms_last = vast_profiler_time_ms();

        if (vast_profiler_entries_count >= VAST_PROFILER_MAX_ENTRIES)
        {
            return;
        }

        vast_profiler_entries[vast_profiler_entries_count++] = entry;
    }
    else
    {
        vast_profiler_entries[entry_id].counter += 1;
        vast_profiler_entries[entry_id].time_ms_last = vast_profiler_time_ms();
    }
}

VAST_API VAST_INLINE void vast_profiler_count(s8 *name, s8 *file, u32 line)
{
    u32 entry_id = vast_profiler_find_entry(name);

    if (entry_id == VAST_PROFILER_ENTRY_INVALID)
    {
        vast_profiler_entry entry = {0};
        entry.name = name;
        entry.file = file;
        entry.line = line;
        entry.counter += 1;

        if (vast_profiler_entries_count >= VAST_PROFILER_MAX_ENTRIES)
        {
            return;
        }

        vast_profiler_entries[vast_profiler_entries_count++] = entry;
    }
    else
    {
        vast_profiler_entries[entry_id].counter += 1;
    }
}

VAST_API VAST_INLINE void vast_profiler_end(s8 *name)
{
    f64 time_ms_end = vast_profiler_time_ms();

    u32 entry_id = vast_profiler_find_entry(name);

    if (entry_id != VAST_PROFILER_ENTRY_INVALID)
    {
        vast_profiler_entry *entry = &vast_profiler_entries[entry_id];

        f64 time_ms_last = time_ms_end - entry->time_ms_last;

        if (entry->counter == 1)
        {
            entry->time_ms_min = time_ms_last;
            entry->time_ms_max = time_ms_last;
        }

        if (time_ms_last < entry->time_ms_min)
        {
            entry->time_ms_min = time_ms_last;
        }

        if (time_ms_last > entry->time_ms_max)
        {
            entry->time_ms_max = time_ms_last;
        }

        entry->time_ms_last = time_ms_last;
        entry->time_ms_total += time_ms_last;
    }
}

#define VAST_PROFILER_BEGIN(name) vast_profiler_begin(#name, __FILE__, __LINE__)
#define VAST_PROFILER_END(name) vast_profiler_end(#name)
#define VAST_PROFILER_COUNT(name) vast_profiler_count(#name, __FILE__, __LINE__)

#endif /* VAST_RELEASE */

#endif /* VAST_PROFILER_H */