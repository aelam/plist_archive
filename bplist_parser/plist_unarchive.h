//
//  bplist_parser.h
//  ideviceruntest
//
//  Created by Wang Lun on 2020/9/28.
//  Copyright © 2020 JonGabilondo. All rights reserved.
//

#ifndef bplist_parser_h
#define bplist_parser_h

#define ENABLE_PRINT 1

#include <plist/plist.h>

int plist_is_archived(plist_t plist);
void plist_unarchive(plist_t plist, plist_t *output);
void print_plist(plist_t pl);

#endif /* bplist_parser_h */
