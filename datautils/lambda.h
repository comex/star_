#pragma once

/*  Example:
    
    int multiplier = 5;
    DECL_LAMBDA(l, int, (int a), {
        return a * multiplier;
    })
    assert(l.func(l.arg, 4) == 20);

    The point of this is to work on both iOS, where GCC inline
    functions don't work, and Linux, where Apple blocks generally
    aren't available.
*/

#ifdef __BLOCKS__
struct _blk {
    void *isa;
    int flags;
    int reserved;
    void *invoke;
};
#define LAMBDA_BODY(typ, ret, args, body) \
    ({ union { \
           ret (^blk) args; \
           struct _blk *_blk; \
           void *vp; \
        } u = { ^ret args body }; \
       (typ) {u._blk->invoke, u.vp}; \
       })
#else
#define LAMBDA_BODY_(typ, ret, args, body) \
    ({ ret func args body; \
       (typ) {&func, 0}; \
       })
#define LAMBDA_BODY(typ, ret, args, body) \
    LAMBDA_BODY_(typ, ret, LAMBDA_UNPAREN args, body)
#endif
#define LAMBDA_UNPAREN(args...) (void *_lambda_ignored, ##args)
#define DECL_LAMBDA(name, ret, args, body) \
    struct { \
        ret (*func) LAMBDA_UNPAREN args; \
        void *arg; \
    } name = LAMBDA_BODY(typeof(name), ret, args, body);
