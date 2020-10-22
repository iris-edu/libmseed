/***************************************************************************
 * A program for reading miniSEED using ms3_readtracelist_timewin() to limit
 * which data is read according to their timestamp. This program also shows
 * how to traverse a trace list.
 *
 * This file is adapted from lm_read_selection.c.
 *
 * Usage: ./lm_read_timewin <mseedfile> <start> <endtime>
 * For example, ./lm_read_timewin test.mseed 2010,058,06,00,00 2010,058,07,00,00
 ***************************************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include <error.h>

#include <libmseed.h>

int main(int argc, char **argv)
{
  MS3TraceList *mstl = NULL;
  MS3TraceID *tid = NULL;
  MS3TraceSeg *seg = NULL;

  char *mseedfile = NULL;
  nstime_t starttime;
  nstime_t endtime;
  char starttimestr[30];
  char endtimestr[30];
  uint32_t flags = 0;
  int8_t verbose = 0;
  int rv;

  /* Parse input parameters */
  if(argc != 4)
  {
    ms_log(2, "Usage: %s <mseedfile> <starttime> <endtime>\n", argv[0]);
    return -1;
  }
  mseedfile = argv[1];
  starttime = ms_timestr2nstime(argv[2]);
  endtime = ms_timestr2nstime(argv[3]);

  /* Set bit flags to validate CRC and unpack data samples */
  flags |= MSF_VALIDATECRC;
  flags |= MSF_UNPACKDATA;

  /* Read all miniSEED into a trace list, limiting from start time to end
   * end time */
  rv = ms3_readtracelist_timewin(&mstl, mseedfile, NULL,
    starttime, endtime,
    0, flags, verbose);
  if(rv != MS_NOERROR)
  {
    ms_log (2, "Cannot read miniSEED from file: %s\n", ms_errorstr(rv));
    return -1;
  }

  /* Traverse trace list structures and print summary information */
  tid = mstl->traces;
  while(tid)
  {
    if (!ms_nstime2timestr (tid->earliest, starttimestr, SEEDORDINAL, NANO_MICRO_NONE) ||
        !ms_nstime2timestr (tid->latest, endtimestr, SEEDORDINAL, NANO_MICRO_NONE))
    {
      ms_log (2, "Cannot create time strings\n");
      starttimestr[0] = endtimestr[0] = '\0';
    }

    ms_log (0, "TraceID for %s (%d), earliest: %s, latest: %s, segments: %u\n",
            tid->sid, tid->pubversion, starttimestr, endtimestr, tid->numsegments);

    seg = tid->first;
    while (seg)
    {
      if (!ms_nstime2timestr (seg->starttime, starttimestr, SEEDORDINAL, NANO_MICRO_NONE) ||
          !ms_nstime2timestr (seg->endtime, endtimestr, SEEDORDINAL, NANO_MICRO_NONE))
      {
        ms_log (2, "Cannot create time strings\n");
        starttimestr[0] = endtimestr[0] = '\0';
      }

      ms_log (0, "  Segment %s - %s, samples: %" PRId64 ", sample rate: %g, sample type: %c\n",
              starttimestr, endtimestr, seg->numsamples, seg->samprate,
              (seg->sampletype) ? seg->sampletype : ' ');

      seg = seg->next;
    }

    tid = tid->next;
  }

  /* Make sure everything is cleaned up */
  if (mstl)
    mstl3_free (&mstl, 0);

  return 0;
}
