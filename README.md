Summary
-------

HAProxy with POC dynamic module support.

Usage
-----

    $ git clone git://github.com/Motiejus/haproxy -b dydy
    $ cd haproxy
    $ make TARGET=generic USE_DL=1 -j5
    $ cc  -fPIC -shared  -o hello-file.so hello-file.c
    $ ./haproxy -f hello_so.conf

In another terminal:

    $ nc localhost 10001
    hello.so

You can see `hello.so` is coming from `hello-file.c`, which is loaded to
haproxy on start time dynamically.

How to navigate the source?
---------------------------

    $ git diff master src/haproxy.c

`dydy_init()` registers callbacks for `dydy-load`, which is found in the
configuration file. Check `hello_so.conf` how it's used.

From there, all the magic happens in `src/dydy.c`. Start at the bottom of the
file.

What still needs to be done
---------------------------

1. Come up with an API for loading dynamic modules. It's OK to copy from Lua.
2. A couple of weeks to implement that API (for someone who knows C).
