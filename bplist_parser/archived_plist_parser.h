//
//  bplist_parser.h
//  ideviceruntest
//
//  Created by Wang Lun on 2020/9/28.
//  Copyright © 2020 JonGabilondo. All rights reserved.
//

#ifndef bplist_parser_h
#define bplist_parser_h

#include <plist/plist.h>

plist_t parse_archived_plist(plist_t plist);
void print_plist(plist_t pl);



#endif /* bplist_parser_h */
