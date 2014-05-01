# This script will get everything setup to make your Yun into a publicly controlable captive portal
# After uploading this script, you will next need to upload the BlinkMe script to make it work. 
# More info at http://wp.josh.com/2014/04/28/a-platform-for…ual-encounters/


# ---------------------- setupSSID
  
  # The SSID is the wifi network name that people will see and be able to connect to

uci set wireless.@wifi-iface[0].ssid='BlinkMe'


# ---------------------- setupDnsmasq
 
  # Add a wildcard DNS enttry so no matter what host they ask for, they will get our IP address...  
  # documented under "-address" here... http:#www.thekelleys.org.uk/dnsmasq/docs/dnsmasq-man.html
  
  # Note that the dest address is a random IP that should land on the WAN side
  # This is fine becuase we capture all outbound HTTP traffic to any address with a firewall rule
  # that sends it to the redirect server. 
  
  # I tried offering the local IP but Andriod phones seem to suppress the setup wizard
  # when the target address is on the same subnet as the phone.
  
uci add_list dhcp.@dnsmasq[0].address='/#/1.1.1.1'
  

# ---------------------- setupFirewall
  
  # Add a new redirect to the firewall to send any HTTP webpage request made to any server besides us
  # to instead go to the local redirect server on port 8080, and that will always
  # send them via a 302 redirect to our normal www server on port 80

  # note that what we really want here is a PREROUTING REDIRECT, not a DNAT since we are REDIRECTING packets to 
  # to the local machine - but I could not figure out how to do that using UCI commands...

uci add firewall redirect  
uci set firewall.@redirect[-1].name=captureHTTP
uci set firewall.@redirect[-1].src=lan
uci set firewall.@redirect[-1].proto=tcp
uci set firewall.@redirect[-1].src_dip=!$(uci get network.lan.ipaddr)
uci set firewall.@redirect[-1].src_dport=80 
uci set firewall.@redirect[-1].dest_port=8080
uci set firewall.@redirect[-1].dest_ip=$(uci get network.lan.ipaddr)
uci set firewall.@redirect[-1].target=DNAT

  # Also add a new redirect to the firewall to send any DNS equest made to any server besides us
  # to instead go to the local DNS server which will always give 1.1.1.1 for any request.
  # This makes things faster on Andriods becuase they seem to ignore the DNS server offered in DHCP
  # and instead go striaght to the Google DNS servers at 8.8.8.8 and 8.8.4.4 on connection...

uci add firewall redirect  
uci set firewall.@redirect[-1].name=captureDNS
uci set firewall.@redirect[-1].src=lan
uci set firewall.@redirect[-1].src_dip=!$(uci get network.lan.ipaddr)
uci set firewall.@redirect[-1].src_dport=53
uci set firewall.@redirect[-1].dest_port=53
uci set firewall.@redirect[-1].dest_ip=$(uci get network.lan.ipaddr)
uci set firewall.@redirect[-1].target=DNAT
  

  
# ---------------------- createRedirectConfig

  # This config file is used only by our secondary HTTP server on port 8080.
 
  # This line will create an alias that will transform all incoming requests that 
  # match '/' (which is *all*) to point instead to our redirection cgi script.

  # This is the only way that I could think of to be able to send an HTTP redirect back on a 
  # request for '/', and it is also nice becuase it automatically matches *any* request
  # so no need to set up an error page on the redirect server
    
echo >/etc/httpd_redirect.conf 'A:/:/cgi-bin/redirect.cgi'

# ---------------------- setupRedirect CGI
  
  # Create a CGI script that will send people to the home page on our normal web server
  # This script is pointed to by the alias in the httpd_redirect.conf that is used by the 
  # secondary uhttpd server running on port 8080. That alias will match *any* incoming URL
  # and send it to this CGI program.

  # Note that this should *not* be cached per RFC 2616 10.3.3, so the user will get the real website 
  # next time they try to pull this up when connected to a real network. Alas, it seems like
  # Firefox mobile was caching it, so we need the cache control headers. 

cat >/www/cgi-bin/redirect.cgi <<EOM  
#!/bin/sh
echo Status: 302 found
echo Location: http://$(uci get network.lan.ipaddr)
echo Cache-Control: no-cache
echo
echo You are headed for http://$(uci get network.lan.ipaddr)
EOM
 
  # Make the new redirect CGI script executable... 
  
chmod +x /www/cgi-bin/redirect.cgi

# ----------------------SetupUhttpdRedirect
  
  # Add a brand new http server on port 8080 that will always just dish out 302 redirects to the primary server.
  # It uses the alias we set up in httpd_redirect.config to send *every* request to redirect.cgi
  # This works with the firewall rule that will send all outbound http requests to this server,
  # which will, in turn, issue an HTTP 302 redirect to the main http server. 
  
uci set uhttpd.redirect='uhttpd'
uci set uhttpd.redirect.listen_http='0.0.0.0:8080'
uci set uhttpd.redirect.cgi_prefix='/cgi-bin'
uci set uhttpd.redirect.config='/etc/httpd_redirect.conf'
uci set uhttpd.redirect.home='/www'


# ---------------------- setupIndex HTML
  
  # Create the index.html file that will get served by the primary web server
  # This file has our very simple user interface page
  # We have to single quote the EOM becuase of the open and close brakets in the HTML
  # More info on how this works here http://www.tldp.org/LDP/abs/html/here-docs.html

cat >/www/index.html <<'EOM'
<!DOCTYPE html><title>Control Me</title><body><center>
<script>
  function send(s) {
     r=new XMLHttpRequest();
     r.open('GET','/cgi-bin/control.cgi?COMMAND='+s+'&'+(new Date()).getTime(),false);
     r.send(null);
  }
</script>
<H1>LED GOES</H1>
<button style='width: 200px;height: 100px;' onclick='send(1);'>On</button>
<button style='width: 200px;height: 100px;' onclick='send(0);'>Off</button>
</center></body>
EOM

# ---------------------- setupControl CGI
  
  # Create the /cgi-bin/control.cgi that is called from the UI at index.html
  # and sends the commands to the Arduino via the tty link
  # Quote the here-document so QUERY_STRING doesn't get evaluated now
  # This may look unsecure and a potential path for an attacker to inject shell
  # commands using the query string, but I think it is OK becuase the variable
  # will only get evaluated once and then sent to the Arduino

cat >/www/cgi-bin/control.cgi <<'EOM'  
#!/bin/sh
echo $QUERY_STRING >/dev/ttyATH0
echo Cache-Control: no-cache
echo Content-type: text/plain
echo 
echo Command sent
EOM


  # Make the new control CGI script executable... 
  
chmod +x /www/cgi-bin/control.cgi


  # Commit all our changes...

uci commit
  
  # And reboot Linino just to be safe (sometimes seems like just doing a restarts leads to an unreachable state)...
  # Must be delayed to give Bridge time to ACK the command back to the Arduino, otherwise the Arduino
  # will keep sending the reboot command over and over again and keep rebooting the Linino forever...

reboot -d 1 &
