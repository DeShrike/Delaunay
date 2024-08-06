#ifndef _DA_H_
#define _DA_H_

// Taken from @Tsoding

#define DA_ASSERT assert
#define DA_REALLOC realloc
#define DA_FREE free

// Initial capacity of a dynamic array
#define DA_INIT_CAP 256

// Append an item to a dynamic array
#define da_append(da, item)                                                              \
    do {                                                                                 \
        if ((da)->count >= (da)->capacity) {                                             \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;       \
            (da)->items = DA_REALLOC((da)->items, (da)->capacity*sizeof(*(da)->items));  \
            DA_ASSERT((da)->items != NULL && "Buy more RAM lol");                        \
        }                                                                                \
                                                                                         \
        (da)->items[(da)->count++] = (item);                                             \
    } while (0)

#define da_free(da) DA_FREE((da).items)

// Append several items to a dynamic array
#define da_append_many(da, new_items, new_items_count)                                      \
    do {                                                                                    \
        if ((da)->count + new_items_count > (da)->capacity) {                               \
            if ((da)->capacity == 0) {                                                      \
                (da)->capacity = DA_INIT_CAP;                                               \
            }                                                                               \
            while ((da)->count + new_items_count > (da)->capacity) {                        \
                (da)->capacity *= 2;                                                        \
            }                                                                               \
            (da)->items = DA_REALLOC((da)->items, (da)->capacity*sizeof(*(da)->items));     \
            DA_ASSERT((da)->items != NULL && "Buy more RAM lol");                           \
        }                                                                                   \
        memcpy((da)->items + (da)->count, new_items, new_items_count*sizeof(*(da)->items)); \
        (da)->count += new_items_count;                                                     \
    } while (0)


#endif // _DA_H_

