BEGIN {print "1..5\n";}
END {print "not ok 1\n" unless $loaded;}
use url;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

$url = "http://www.dummy.fr/foo?bar";
$url_object = url_alloc($url, length($url));

$url eq url_url($url_object) ? print "ok 2\n" : print "not ok 2\n";

$url_object->{'path'} eq 'foo' ? print "ok 3\n" : print "not ok 3 \n";

url_free($url_object);
print "ok 4\n";

$url = "http://www,dummy!fr/";
$url_object = url_alloc($url, length($url));
!defined($url_object) ? print "ok 5\n" : print "not ok 5 \n";
