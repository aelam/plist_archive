//
//  bplist_parser.c
//  ideviceruntest
//
//  Created by Wang Lun on 2020/9/28.
//  Copyright © 2020 JonGabilondo. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archived_plist_parser.h"

/// MARK: DEBUG
#if ENABLE_PRINT
void printNode(plist_t node) {
    plist_type t = plist_get_node_type(node);
    if (t == PLIST_UINT) {
        uint64_t v = 0;
        plist_get_uint_val(node, &v);
        printf("%lld", v);
    } else if (t == PLIST_REAL) {
        double v = 0;
        plist_get_real_val(node, &v);
        printf("%f", v);
    } else if (t == PLIST_DICT) {
        printf("A DIC");
    } else if (t == PLIST_ARRAY) {
        printf("AN ARRAY");
    } else if (t == PLIST_STRING) {
        char *v;
        plist_get_string_val(node, &v);
        printf("%s", v);
    } else if (t == PLIST_BOOLEAN) {
        uint8_t v;
        plist_get_bool_val(node, &v);
        printf("%d", v);
    }
}

void print_plist(plist_t pl) {
    char* xml = NULL;
    uint32_t xlen = 0;
    plist_to_xml(pl, &xml, &xlen);

    if (xml) {
        printf("%s\n", xml);
        free(xml);
    }
}
#else
void printNode(plist_t node) {}
void print_plist(plist_t pl) {}

#endif


/// MARK: PRIVATE API
char *parse_bplist_object_key(plist_t objects, plist_t current_node_key);
plist_t parse_bplist_UID_object(plist_t objects, plist_t current_node);
plist_t parse_archived_plist_dictionary(plist_t objects, plist_t data);
plist_t parse_archived_plist_array(plist_t objects, plist_t data);
plist_t parse_bplist_object(plist_t objects, plist_t data);


typedef enum : uint32_t {
    archived_plist_data_type_dictionary,
    archived_plist_data_type_array,
    archived_plist_data_type_date,
    archived_plist_data_type_unknown
} archived_plist_data_type;

uint64_t plist_get_UID_val(plist_t node) {
    uint64_t uid_val = 0;
    plist_type type = plist_get_node_type(node);
    if (type == PLIST_UINT) {
        plist_get_uint_val(node, &uid_val);
    } else if (type == PLIST_UID) {
        plist_get_uid_val(node, &uid_val);
    } else if (type == PLIST_REAL) {
        double uid = 0;
        plist_get_real_val(node, &uid);
        uid_val = uid;
    }
    return uid_val;
}


archived_plist_data_type parse_class_type_uid(plist_t objects, uint32_t classTypeUID) {
    plist_t item = plist_array_get_item(objects, classTypeUID);
    plist_t data_type_classname = plist_dict_get_item(item, "$classname");
    char * data_type_classname_val;
    plist_get_string_val(data_type_classname, &data_type_classname_val);

    if(strcmp(data_type_classname_val, "NSMutableDictionary") == 0 ||
       strcmp(data_type_classname_val, "NSDictionary") == 0
       ) {
        return archived_plist_data_type_dictionary;
    } else if(strcmp(data_type_classname_val, "NSMutableArray") == 0 ||
              strcmp(data_type_classname_val, "NSArray") == 0
              ) {
        return archived_plist_data_type_array;
    } else if (strcmp(data_type_classname_val, "NSDate") == 0) {
        return archived_plist_data_type_date;
    } else {
        return archived_plist_data_type_unknown;
    }
}

int plist_is_archived(plist_t plist) {
    plist_t *archive_node = plist_dict_get_item(plist, "$archiver");
    if (archive_node == NULL) {
        return 0;
    }
    
    char *s = NULL;
    plist_get_string_val(archive_node, &s);
    return strcmp(s, "NSKeyedArchiver") == 0;
}

// Object Key
// Read from Array of NS.keys
char *parse_bplist_object_key(plist_t objects, plist_t current_node_key) {
    plist_t keyUID = plist_dict_get_item(current_node_key, "CF$UID");
    uint64_t data_type_uid_val = plist_get_UID_val(keyUID);
    plist_t t = plist_array_get_item(objects, (uint32_t)data_type_uid_val);
    char *result = NULL;
    plist_get_string_val(t, &result);
    
    return result;
}

/// MARK: parse Dictionary, Date, Array without UID
plist_t parse_archived_plist_dictionary(plist_t objects, plist_t data) {
    plist_t new_plist = plist_new_dict();

    plist_t data_keys = plist_access_path(data, 1, "NS.keys");
    plist_t data_objects = plist_access_path(data, 1, "NS.objects");
    uint32_t size = plist_array_get_size(data_keys);
    
    for (int i = 0; i < size; i++) {
        plist_t keys_key = plist_array_get_item(data_keys, i);
        char *key = parse_bplist_object_key(objects, keys_key);
        
        plist_t value_node = plist_array_get_item(data_objects, i);
        plist_t value = parse_bplist_UID_object(objects, value_node);
        
        plist_dict_set_item(new_plist, key, value);
    }

    return new_plist;

}

plist_t parse_archived_plist_date(plist_t objects, plist_t date) {
    plist_t node_time = plist_access_path(date, 1, "NS.time");
    return plist_copy(node_time);
}

plist_t parse_archived_plist_array(plist_t objects, plist_t data) {
    plist_t new_plist = plist_new_array();
    plist_t data_array_objects = plist_access_path(data, 1, "NS.objects");
    uint32_t objects_size = plist_array_get_size(data_array_objects);

    for (uint32_t i = 0; i < objects_size; i++ ) {
        plist_t current_node = plist_array_get_item(data_array_objects, i);
        plist_t item_node = parse_bplist_UID_object(objects, current_node);
        plist_array_append_item(new_plist, item_node);
    }
    return new_plist;
}

plist_t parse_bplist_object(plist_t objects, plist_t data) {
    plist_t new_plist = NULL;
    
    plist_t data_type_uid = plist_access_path(data, 2, "$class", "CF$UID");
    uint64_t data_type_uid_val = plist_get_UID_val(data_type_uid);
    
    {
        archived_plist_data_type type = parse_class_type_uid(objects, (uint32_t)data_type_uid_val);
        if (type == archived_plist_data_type_dictionary) {
            new_plist = parse_archived_plist_dictionary(objects, data);
        } else if (type == archived_plist_data_type_array) {
            new_plist = parse_archived_plist_array(objects, data);
        } else if (type == archived_plist_data_type_date) {
            new_plist = parse_archived_plist_date(objects, data);
        } else {
            
        }
        return new_plist;
    }
}

/// MARK: parse Dictionary, Date, Array WITH UID
plist_t parse_bplist_UID_object(plist_t objects, plist_t uid_node) {
    plist_t data_uid = plist_access_path(uid_node, 1, "CF$UID");
    uint64_t data_uid_val = plist_get_UID_val(data_uid);
    plist_t data = plist_array_get_item(objects, (uint32_t)data_uid_val);
    plist_type type = plist_get_node_type(data);
    
    if (type ==         PLIST_BOOLEAN ||
        type ==         PLIST_UINT ||
        type ==         PLIST_REAL ||
        type ==         PLIST_STRING) {
        return plist_copy(data);
    } else if (type == PLIST_DICT) {
        return parse_bplist_object(objects, data);
    }
    return plist_copy(data);

//    return parse_bplist_object(objects, data);
}

// API
plist_t parse_archived_plist(plist_t bplist) {
    if (plist_is_archived(bplist) == 0) {
        return bplist;
    }
    
    // root type
    plist_t root_uid = plist_access_path(bplist, 3, "$top", "root", "CF$UID");
    plist_t objects = plist_dict_get_item(bplist, "$objects");
    
    // get root Id
    uint64_t root_uid_val = plist_get_UID_val(root_uid);

    // get root class id
    plist_t data = plist_array_get_item(objects, (uint32_t)root_uid_val);
    
    return parse_bplist_object(objects, data);
}

