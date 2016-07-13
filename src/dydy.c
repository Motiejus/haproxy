#include <common/cfgparse.h>

static int dydy_load(char **args, int section_type, struct proxy *curpx,
                     struct proxy *defpx, const char *file, int line,
                     char **err)
{
    fprintf(stderr, "started dydy_load...\n");
    return 0;
}

static struct cfg_kw_list cfg_kws = {{ },{
	{ CFG_GLOBAL, "dydy-load", dydy_load },
	{ 0, NULL, NULL },
}};

void dydy_init(void)
{
    cfg_register_keywords(&cfg_kws);
}
