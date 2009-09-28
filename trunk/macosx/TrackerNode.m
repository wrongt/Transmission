/******************************************************************************
 * $Id$
 *
 * Copyright (c) 2009 Transmission authors and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#import "TrackerNode.h"
#import "NSApplicationAdditions.h"
#import "NSStringAdditions.h"

@implementation TrackerNode

- (id) initWithTrackerStat: (tr_tracker_stat *) stat
{
    if ((self = [super init]))
    {
        fStat = *stat;
    }
    
    return self;
}

- (id) copyWithZone: (NSZone *) zone
{
    //this object is essentially immutable after initial setup
    return [self retain];
}

- (NSString *) host
{
    return [NSString stringWithUTF8String: fStat.host];
}

#warning consider "isActive"
- (NSString *) lastAnnounceStatusString
{
    NSString * dateString;
    if (fStat.hasAnnounced && fStat.lastAnnounceTime != 0)
    {
        NSDateFormatter * dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateStyle: NSDateFormatterFullStyle];
        [dateFormatter setTimeStyle: NSDateFormatterShortStyle];
        
        if ([NSApp isOnSnowLeopardOrBetter])
            [dateFormatter setDoesRelativeDateFormatting: YES];
        
        dateString = [dateFormatter stringFromDate: [NSDate dateWithTimeIntervalSince1970: fStat.lastAnnounceTime]];
        [dateFormatter release];
    }
    else
        dateString = NSLocalizedString(@"N/A", "Tracker last announce");
    
    if (fStat.hasAnnounced && !fStat.lastAnnounceSucceeded)
        dateString = [NSString stringWithFormat: @"%@: %@ - %@", NSLocalizedString(@"Announce error", "Tracker last announce"),
                        [NSString stringWithUTF8String: fStat.lastAnnounceResult], dateString];
    else
    {
        dateString = [NSString stringWithFormat: NSLocalizedString(@"Last Announce: %@", "Tracker last announce"),
                        dateString];
        if (fStat.hasAnnounced && fStat.lastAnnounceSucceeded)
            dateString = [dateString stringByAppendingFormat: NSLocalizedString(@" (got %d peers)", "Tracker last announce"),
                            fStat.lastAnnouncePeerCount];
    }
    
    return dateString;
}

- (NSString *) nextAnnounceStatusString
{
    if (fStat.isAnnouncing)
        return [NSLocalizedString(@"Announce in progress", "Tracker next announce") stringByAppendingEllipsis];
    else if (fStat.willAnnounce)
        return [NSString stringWithFormat: NSLocalizedString(@"Next announce in %@", "Tracker next announce"),
                [NSString timeString: fStat.nextAnnounceTime - [[NSDate date] timeIntervalSince1970] showSeconds: YES]];
    else
        return NSLocalizedString(@"Announce not scheduled", "Tracker next announce");
}

- (NSString *) lastScrapeStatusString
{
    NSString * dateString;
    if (fStat.hasScraped && fStat.lastScrapeTime != 0)
    {
        NSDateFormatter * dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateStyle: NSDateFormatterFullStyle];
        [dateFormatter setTimeStyle: NSDateFormatterShortStyle];
        
        if ([NSApp isOnSnowLeopardOrBetter])
            [dateFormatter setDoesRelativeDateFormatting: YES];
        
        dateString = [dateFormatter stringFromDate: [NSDate dateWithTimeIntervalSince1970: fStat.lastScrapeTime]];
        [dateFormatter release];
    }
    else
        dateString = NSLocalizedString(@"N/A", "Tracker last announce");
    
    if (fStat.hasScraped && !fStat.lastScrapeSucceeded)
        dateString = [NSString stringWithFormat: @"%@: %@ - %@", NSLocalizedString(@"Scrape error", "Tracker last announce"),
                        [NSString stringWithUTF8String: fStat.lastScrapeResult], dateString];
    else
        dateString = [NSString stringWithFormat: NSLocalizedString(@"Last Scrape: %@", "Tracker last announce"), dateString];
    
    return dateString;
}

@end
