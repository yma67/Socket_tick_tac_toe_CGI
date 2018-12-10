# Socket_tick_tac_toe_CGI
Tick Tac Toe CGI implementation using C

``
url: https://www.cs.mcgill.ca/~yma67/ttt.html
``

``
port: 62223
``

``
ip: 132.206.51.22
``

# To start backend server
``
./server 62224
``

# To start command line client
``
./client 127.0.0.1 62224
``
(if you are using your own computer as server)

# To deploy cgi
``
gcc ttt_cgi.c -o YOUR_APACHE_DIRECTORY/cgi-bin/ttt.cgi
``
(if you are using your own computer as server)
