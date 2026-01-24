/*
 * Portable timegm() implementation for IRIX
 *
 * timegm is a GNU extension that converts a struct tm to time_t
 * assuming the time is in UTC (as opposed to mktime which uses local time).
 *
 * This implementation avoids setenv/unsetenv which may not exist on IRIX.
 * Instead it uses a direct calculation approach.
 *
 * Licensed under 3-clause BSD license.
 */

#include <time.h>

/*
 * timegm - Convert a broken-down time to time_t in UTC
 *
 * This is the inverse of gmtime(). Unlike mktime(), timegm()
 * interprets the input as UTC, not local time.
 *
 * Implementation calculates seconds since epoch directly.
 */
time_t timegm(struct tm *tm)
{
    static const int days_in_month[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    time_t days;
    int year, month, i;

    year = tm->tm_year + 1900;
    month = tm->tm_mon;

    /* Days from epoch (1970-01-01) to start of this year */
    days = (year - 1970) * 365;

    /* Add leap years from 1970 to this year */
    for (i = 1970; i < year; i++) {
        if ((i % 4 == 0 && i % 100 != 0) || (i % 400 == 0))
            days++;
    }

    /* Add days in months of this year */
    for (i = 0; i < month; i++) {
        days += days_in_month[i];
        /* Add leap day if in a leap year and past February */
        if (i == 1 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
            days++;
    }

    /* Add day of month (tm_mday is 1-based) */
    days += tm->tm_mday - 1;

    /* Convert to seconds and add time */
    return (days * 86400) + (tm->tm_hour * 3600) + (tm->tm_min * 60) + tm->tm_sec;
}
