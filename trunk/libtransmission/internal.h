/******************************************************************************
 * $Id$
 *
 * Copyright (c) 2005-2007 Transmission authors and contributors
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

#ifndef TR_INTERNAL_H
#define TR_INTERNAL_H 1


#ifdef SYS_BEOS
/* BeOS doesn't declare vasprintf in its headers, but actually
 * implements it */
extern int vasprintf( char **, const char *, va_list );
#include <signal.h>
#endif

#ifdef SYS_BEOS
#  define socklen_t uint32_t
#endif

#if defined(BEOS_NETSERVER) || defined(__MINGW__)
#include <stdint.h>
typedef uint16_t tr_port_t;
#else
#include <arpa/inet.h>
typedef in_port_t tr_port_t;
#endif

#define TR_NAME "Transmission"

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

#ifdef __GNUC__
#  define UNUSED __attribute__((unused))
#  define PRINTF( fmt, args ) __attribute__((format (printf, fmt, args)))
#else
#  define UNUSED
#  define PRINTF( fmt, args )
#endif

#define TR_MAX_PEER_COUNT 60

typedef enum { TR_NET_OK, TR_NET_ERROR, TR_NET_WAIT } tr_tristate_t;

#include "platform.h" /* thread, lock */
#include "peer.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

void tr_peerIdNew ( char* buf, int buflen );

void tr_torrentResetTransferStats( tr_torrent_t * );

int tr_torrentAddCompact( tr_torrent_t * tor, int from,
                           uint8_t * buf, int count );
int tr_torrentAttachPeer( tr_torrent_t * tor, tr_peer_t * );

void tr_torrentSetHasPiece( tr_torrent_t * tor, int pieceIndex, int has );

void tr_torrentReaderLock    ( const tr_torrent_t * );
void tr_torrentReaderUnlock  ( const tr_torrent_t * );
void tr_torrentWriterLock    ( tr_torrent_t * );
void tr_torrentWriterUnlock  ( tr_torrent_t * );

/* get the index of this piece's first block */
#define tr_torPieceFirstBlock(tor,piece) ( (piece) * (tor)->blockCountInPiece )

/* what piece index is this block in? */
#define tr_torBlockPiece(tor,block) ( (block) / (tor)->blockCountInPiece )

/* how many blocks are in this piece? */
#define tr_torPieceCountBlocks(tor,piece) \
    ( ((piece)==((tor)->info.pieceCount-1)) ? (tor)->blockCountInLastPiece : (tor)->blockCountInPiece )

/* how many bytes are in this piece? */
#define tr_torPieceCountBytes(tor,piece) \
    ( ((piece)==((tor)->info.pieceCount-1)) ? (tor)->lastPieceSize : (tor)->info.pieceSize )

/* how many bytes are in this block? */
#define tr_torBlockCountBytes(tor,block) \
    ( ((block)==((tor)->blockCount-1)) ? (tor)->lastBlockSize : (tor)->blockSize )

#define tr_block(a,b) _tr_block(tor,a,b)
int _tr_block( const tr_torrent_t * tor, int index, int begin );


typedef enum
{
    TR_RUN_CHECKING           = (1<<0), /* checking files' checksums */
    TR_RUN_RUNNING            = (1<<1), /* seeding or leeching */
    TR_RUN_STOPPING           = (1<<2), /* stopping */
    TR_RUN_STOPPING_NET_WAIT  = (1<<3), /* waiting on network -- we're
                                           telling tracker we've stopped */
    TR_RUN_STOPPED            = (1<<4)  /* stopped */
}
run_status_t;

#define TR_ID_LEN               20
#define TR_KEY_LEN              20

struct tr_torrent_s
{
    tr_handle_t * handle;
    tr_info_t info;

    tr_speedlimit_t    uploadLimitMode;
    tr_speedlimit_t    downloadLimitMode;
    struct tr_ratecontrol_s * upload;
    struct tr_ratecontrol_s * download;
    struct tr_ratecontrol_s * swarmspeed;

    int               error;
    char              errorString[128];
    int               hasChangedState;

    char              peer_id[TR_ID_LEN+1];
    char            * key;
    uint8_t         * azId;
    int               publicPort;

    /* An escaped string used to include the hash in HTTP queries */
    char              escapedHashString[3*SHA_DIGEST_LENGTH+1];

    /* Where to download */
    char            * destination;
    
    /* How many bytes we ask for per request */
    int               blockSize;
    int               blockCount;

    int               lastBlockSize;
    int               lastPieceSize;

    int               blockCountInPiece;
    int               blockCountInLastPiece;
    
    struct tr_completion_s * completion;

    volatile char     dieFlag;
    struct tr_bitfield_s   * uncheckedPieces;
    run_status_t      runStatus;
    cp_status_t       cpStatus;
    tr_thread_t     * thread;
    tr_rwlock_t     * lock;

    struct tr_tracker_s    * tracker;
    struct tr_io_s         * io;
    uint64_t          startDate;
    uint64_t          stopDate;
    char              ioLoaded;
    char              fastResumeDirty;

    int               peerCount;
    tr_peer_t       * peers[TR_MAX_PEER_COUNT];

    uint64_t          downloadedCur;
    uint64_t          downloadedPrev;
    uint64_t          uploadedCur;
    uint64_t          uploadedPrev;
    uint64_t          activityDate;

    uint8_t           pexDisabled;

    int8_t            statCur;
    tr_stat_t         stats[2];

    tr_torrent_t    * next;
};

struct tr_handle_s
{
    int                torrentCount;
    tr_torrent_t     * torrentList;

    char             * tag;
    int                isPortSet;

    char               useUploadLimit;
    char               useDownloadLimit;
    struct tr_ratecontrol_s * upload;
    struct tr_ratecontrol_s * download;

    struct tr_shared_s      * shared;

    char           key[TR_KEY_LEN+1];

    tr_handle_status_t stats[2];
    int                statCur;
#define TR_AZ_ID_LEN            20
    uint8_t        azId[TR_AZ_ID_LEN];
};

#endif
