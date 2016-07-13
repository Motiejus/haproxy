#include <common/cfgparse.h>
#include <proto/proto_tcp.h>
#include <proto/stream.h>
#include <proto/stream_interface.h>

static unsigned int dydy_timeout_applet = 4000; /* applet timeout. */

static void dydy_applet_tcp_fct(struct appctx *ctx) {
	/* If the stream is disconnect or closed, ldo nothing. */
    fprintf(stderr, "applet invoked...\n");

	struct stream_interface *si = ctx->owner;
	struct channel *chn = si_ic(si);
	size_t len = strlen("hello\n");
	int max;

	if (unlikely(si->state == SI_ST_DIS || si->state == SI_ST_CLO))
		return;

	/* Execute the function. */
	max = channel_recv_max(chn);
	if (max > (len))
		max = len;
	bi_putblk(chn, "hello\n", max);

	si_shutw(si);
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
	rule->applet.timeout = dydy_timeout_applet;

	return 0;
}

static int dydy_load(char **args, int section_type, struct proxy *curpx,
                     struct proxy *defpx, const char *file, int line,
                     char **err)
{
    fprintf(stderr, "started dydy_load...\n");
	struct action_kw_list *akl;
	const char *name = "dydy";
	int len;

	akl = calloc(1, sizeof(*akl) + sizeof(struct action_kw) * 2);

	/* List head */
	akl->list.n = akl->list.p = NULL;

	len = strlen("dydy.") + strlen(name) + 1;
	akl->kw[0].kw = calloc(1, len);

	snprintf((char *)akl->kw[0].kw, len, "dydy.%s", name);

	akl->kw[0].parse = action_register_service_tcp;
	akl->kw[0].match_pfx = 0;
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
