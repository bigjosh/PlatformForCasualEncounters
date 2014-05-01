/*

 This sketch will run a bunch of linux commands on an Arduino Yun to set it up
 to be a captive portal as described in:
 http://wp.josh.com/platform-for-casual-encounters
 
 After uploading this sketch, wait for the red LED on the board to start blinking at a steady 
 1 blink per second to know that the setup is complete. Then upload the BlinkMe sketch.
 
 The sending of shell commands based on example code from:
 http://arduino.cc/en/Tutorial/ShellCommands
 
 The use of the F() macro to keep strings in flash described here:
 http://stackoverflow.com/questions/16597437/arduino-f-what-does-it-actually-do

 */
 
#include <Process.h>

// Actually run a shell command on the Linino using the Bridge...

void run( const __FlashStringHelper* commandString) {
  
    Process p;
    
    Serial.println( F("Command:") );     
    Serial.println( commandString );

    digitalWrite( 13 , HIGH );      
    Serial.println( F("Running...") );    
    p.runShellCommand( commandString );    
        
    Serial.println( F("Done...") );
    
    digitalWrite( 13 , LOW );  

    delay(100);    
}

void setup() {
  
  Serial.begin(9600);	// Initialize the Serial  
  delay(5000);          // Give people some time to connect via serial monitor...
  
  Serial.println(F("Starting Bridge..."));
  Bridge.begin();	// Initialize the Bridge
     
// What follows is code automatically generated from the shell script BlinkMeSetup.sh
// ==========================================================================================================

#define NL "\n"

// This script will get everything setup to make your Yun into a publicly controlable captive portal
// After uploading this script, you will next need to upload the BlinkMe script to make it work. 
// More info at http://wp.josh.com/2014/04/28/a-platform-for…ual-encounters/


// ---------------------- setupSSID

//   The SSID is the wifi network name that people will see and be able to connect to

   run( F("uci set wireless.@wifi-iface[0].ssid='BlinkMe'"));


// ---------------------- setupDnsmasq

//   Add a wildcard DNS enttry so no matter what host they ask for, they will get our IP address...  
//   documented under "-address" here... http:www.thekelleys.org.uk/dnsmasq/docs/dnsmasq-man.html

//   Note that the dest address is a random IP that should land on the WAN side
//   This is fine becuase we capture all outbound HTTP traffic to any address with a firewall rule
//   that sends it to the redirect server. 

//   I tried offering the local IP but Andriod phones seem to suppress the setup wizard
//   when the target address is on the same subnet as the phone.

   run( F("uci add_list dhcp.@dnsmasq[0].address='/#/1.1.1.1'"));


// ---------------------- setupFirewall

//   Add a new redirect to the firewall to send any HTTP webpage request made to any server besides us
//   to instead go to the local redirect server on port 8080, and that will always
//   send them via a 302 redirect to our normal www server on port 80

//   note that what we really want here is a PREROUTING REDIRECT, not a DNAT since we are REDIRECTING packets to 
//   to the local machine - but I could not figure out how to do that using UCI commands...

   run( F("uci add firewall redirect  "));
   run( F("uci set firewall.@redirect[-1].name=captureHTTP"));
   run( F("uci set firewall.@redirect[-1].src=lan"));
   run( F("uci set firewall.@redirect[-1].proto=tcp"));
   run( F("uci set firewall.@redirect[-1].src_dip=!$(uci get network.lan.ipaddr)"));
   run( F("uci set firewall.@redirect[-1].src_dport=80"));
   run( F("uci set firewall.@redirect[-1].dest_port=8080"));
   run( F("uci set firewall.@redirect[-1].dest_ip=$(uci get network.lan.ipaddr)"));
   run( F("uci set firewall.@redirect[-1].target=DNAT"));

//   Also add a new redirect to the firewall to send any DNS equest made to any server besides us
//   to instead go to the local DNS server which will always give 1.1.1.1 for any request.
//   This makes things faster on Andriods becuase they seem to ignore the DNS server offered in DHCP
//   and instead go striaght to the Google DNS servers at 8.8.8.8 and 8.8.4.4 on connection...

   run( F("uci add firewall redirect  "));
   run( F("uci set firewall.@redirect[-1].name=captureDNS"));
   run( F("uci set firewall.@redirect[-1].src=lan"));
   run( F("uci set firewall.@redirect[-1].src_dip=!$(uci get network.lan.ipaddr)"));
   run( F("uci set firewall.@redirect[-1].src_dport=53"));
   run( F("uci set firewall.@redirect[-1].dest_port=53"));
   run( F("uci set firewall.@redirect[-1].dest_ip=$(uci get network.lan.ipaddr)"));
   run( F("uci set firewall.@redirect[-1].target=DNAT"));



// ---------------------- createRedirectConfig

//   This config file is used only by our secondary HTTP server on port 8080.

//   This line will create an alias that will transform all incoming requests that 
//   match '/' (which is *all*) to point instead to our redirection cgi script.

//   This is the only way that I could think of to be able to send an HTTP redirect back on a 
//   request for '/', and it is also nice becuase it automatically matches *any* request
//   so no need to set up an error page on the redirect server

   run( F("echo >/etc/httpd_redirect.conf 'A:/:/cgi-bin/redirect.cgi'"));

// ---------------------- setupRedirect CGI

//   Create a CGI script that will send people to the home page on our normal web server
//   This script is pointed to by the alias in the httpd_redirect.conf that is used by the 
//   secondary uhttpd server running on port 8080. That alias will match *any* incoming URL
//   and send it to this CGI program.

//   Note that this should *not* be cached per RFC 2616 10.3.3, so the user will get the real website 
//   next time they try to pull this up when connected to a real network. Alas, it seems like
//   Firefox mobile was caching it, so we need the cache control headers. 

   run( F("cat >/www/cgi-bin/redirect.cgi <<EOM  " NL
   //-----
   "#!/bin/sh" NL
   "echo Status: 302 found" NL
   "echo Location: http://$(uci get network.lan.ipaddr)" NL
   "echo Cache-Control: no-cache" NL
   "echo" NL
   "echo You are headed for http://$(uci get network.lan.ipaddr)" NL
   //-----
    "EOM" NL));

//   Make the new redirect CGI script executable... 

   run( F("chmod +x /www/cgi-bin/redirect.cgi"));

// ----------------------SetupUhttpdRedirect

//   Add a brand new http server on port 8080 that will always just dish out 302 redirects to the primary server.
//   It uses the alias we set up in httpd_redirect.config to send *every* request to redirect.cgi
//   This works with the firewall rule that will send all outbound http requests to this server,
//   which will, in turn, issue an HTTP 302 redirect to the main http server. 

   run( F("uci set uhttpd.redirect='uhttpd'"));
   run( F("uci set uhttpd.redirect.listen_http='0.0.0.0:8080'"));
   run( F("uci set uhttpd.redirect.cgi_prefix='/cgi-bin'"));
   run( F("uci set uhttpd.redirect.config='/etc/httpd_redirect.conf'"));
   run( F("uci set uhttpd.redirect.home='/www'"));


// ---------------------- setupIndex HTML

//   Create the index.html file that will get served by the primary web server
//   This file has our very simple user interface page
//   We have to single quote the EOM becuase of the open and close brakets in the HTML
//   More info on how this works here http://www.tldp.org/LDP/abs/html/here-docs.html

   run( F("cat >/www/index.html <<'EOM'" NL
   //-----
   "<!DOCTYPE html><title>Control Me</title><body><center>" NL
   "<script>" NL
   "  function send(s) {" NL
   "     r=new XMLHttpRequest();" NL
   "     r.open('GET','/cgi-bin/control.cgi?COMMAND='+s+'&'+(new Date()).getTime(),false);" NL
   "     r.send(null);" NL
   "  }" NL
   "</script>" NL
   "<H1>LED GOES</H1>" NL
   "<button style='width: 200px;height: 100px;' onclick='send(1);'>On</button>" NL
   "<button style='width: 200px;height: 100px;' onclick='send(0);'>Off</button>" NL
   "</center></body>" NL
   //-----
    "EOM" NL));

// ---------------------- setupControl CGI

//   Create the /cgi-bin/control.cgi that is called from the UI at index.html
//   and sends the commands to the Arduino via the tty link
//   Quote the here-document so QUERY_STRING doesn't get evaluated now
//   This may look unsecure and a potential path for an attacker to inject shell
//   commands using the query string, but I think it is OK becuase the variable
//   will only get evaluated once and then sent to the Arduino

   run( F("cat >/www/cgi-bin/control.cgi <<'EOM'  " NL
   //-----
   "#!/bin/sh" NL
   "echo $QUERY_STRING >/dev/ttyATH0" NL
   "echo Cache-Control: no-cache" NL
   "echo Content-type: text/plain" NL
   "echo " NL
   "echo Command sent" NL
   //-----
    "EOM" NL));


//   Make the new control CGI script executable... 

   run( F("chmod +x /www/cgi-bin/control.cgi"));


//   Commit all our changes...

   run( F("uci commit"));

//   And reboot Linino just to be safe (sometimes seems like just doing a restarts leads to an unreachable state)...
//   Must be delayed to give Bridge time to ACK the command back to the Arduino, otherwise the Arduino
//   will keep sending the reboot command over and over again and keep rebooting the Linino forever...

   run( F("reboot -d 1 &"));

// ==========================================================================================================
// End of code automatically generated from the shell script BlinkMeSetup.sh

  Serial.println(F("Setup complete. Now upload the BlinkMe.ino sketch."));

}


void loop() {
  
    
  // Signal all done with a 1Hz blinking LED
  
  digitalWrite( 13 , LOW );
  delay(500);
  digitalWrite( 13 , HIGH );
  delay(500);  
  
}



