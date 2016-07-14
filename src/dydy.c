#include <dlfcn.h>

#include <common/cfgparse.h>
#include <proto/proto_tcp.h>
#include <proto/stream.h>
#include <proto/stream_interface.h>

static unsigned int dydy_timeout_applet = 4000; /* applet timeout. */

static char* (*user_fn)(void); /* function which returns "hello" */

static int dydy_applet_tcp_init(struct appctx *ctx, struct proxy *px, struct stream *strm) {
	struct stream_interface *si = ctx->owner;

	/* Wakeup the applet ASAP. */
	si_applet_cant_get(si);
	si_applet_cant_put(si);

	return 1;
}

static void dydy_applet_tcp_fct(struct appctx *ctx) {
	struct stream_interface *si;
	struct channel *chn;
	struct channel *res;
	int max;
	char *str = user_fn();
	//char *str = "hello.o!\n";
	size_t len = strlen(str);

	si = ctx->owner;
	chn = si_ic(si);
	res = si_ic(si);

	/* If the stream is disconnect or closed, ldo nothing. */
	if (unlikely(si->state == SI_ST_DIS || si->state == SI_ST_CLO))
		return;

	/* Execute the function. */
	max = channel_recv_max(chn);
    fprintf(stderr, "max: %d, len: %d\n", max, (int)len);
	if (max > (len))
		max = len;
	bi_putblk(chn, str, max);

	/* eat the whole request */
	bo_skip(si_oc(si), si_ob(si)->o);
	res->flags |= CF_READ_NULL;
	si_shutr(si);
}


static enum act_parse_ret action_register_service_tcp(
		const char **args, int *cur_arg, struct proxy *px,
		struct act_rule *rule, char **err) {
    fprintf(stderr, "registering dydy tcp service...\n");

	/* Add applet pointer in the rule. */
	rule->applet.obj_type = OBJ_TYPE_APPLET;
	rule->applet.name = "dydy";
	rule->applet.fct = dydy_applet_tcp_fct;
	rule->applet.init = dydy_applet_tcp_init;
	rule->applet.timeout = dydy_timeout_applet;

	return 0;
}

static int dydy_load(char **args, int section_type, struct proxy *curpx,
                     struct proxy *defpx, const char *file, int line,
                     char **err)
{
	struct action_kw_list *akl;
	int len;
	void *handle; // dynamic library
	const char *name = "stub";
    fprintf(stderr, "loading %s...\n", args[1]);

	akl = calloc(1, sizeof(*akl) + sizeof(struct action_kw) * 2);

	/* List head */
	akl->list.n = akl->list.p = NULL;

	/* converter keyword */
	len = strlen("dydy.") + strlen(name) + 1;
	akl->kw[0].kw = calloc(1, len);

	snprintf((char *)akl->kw[0].kw, len, "dydy.%s", name);

	akl->kw[0].parse = action_register_service_tcp;
	akl->kw[0].match_pfx = 0;

	/* Load the dynamic library */
	handle = dlopen(args[1], RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror();    /* Clear any existing error */
	*(void **) (&user_fn) = dlsym(handle, "user_fn");

	akl->kw[0].private = NULL;

	/* End of array. */
	memset(&akl->kw[1], 0, sizeof(*akl->kw));

	/* Register this new converter */
	service_keywords_register(akl);

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
