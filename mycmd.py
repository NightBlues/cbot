import os
import sys

id_ = os.fork()
if id_ != 0:
    print "Exiting from parent, proc id = {}".format(id_)
    sys.exit(0)
os.setsid();
map(os.close, [0, 1, 2])
import SimpleHTTPServer
SimpleHTTPServer.test()
