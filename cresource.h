/* mkcres
 * https://github.com/jahnf/mkcres
 * For licensing and details see LICENSE and README.md
 */
 
#ifndef CRESOURCE__H_
#define CRESOURCE__H_

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct {
   const char* const name;
   const unsigned long size;
   const unsigned char *data;
} cresource_t;

typedef const struct {
    const char* const prefix;
    const unsigned int prefix_len;
    const unsigned long num_resources;
    const cresource_t* const resources[];
} cresource_prefix_t;

typedef const struct {
    const unsigned long num_prefix_sections;
    const cresource_prefix_t* const prefix_sections[];
} cresource_collection_t;

/** Get a resource with the given filename. Returns a null ptr if the resource
 * was not found. */
cresource_t* get_cresource(const char* filename);

/** Get resource collection for the application. The implementation of this 
 * function is generated by cresource generator together with the resources. */
cresource_collection_t* get_cresources();

#ifdef __cplusplus
}
#endif

#endif /* CRESOURCE__H_ */
