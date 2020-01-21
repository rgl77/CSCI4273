CSCI4273
Programming Assignment 3
------------------------
Fall 2019
Orgil Sugar
------------

Instructions to run the code:
1.->make
2.->./webproxy <port_number> <timeout_preference>


I have implemented the timeout setting and basic caching by saving the visited addresses on the program. The cache would be a linked list and everytime there is a new page that we've never visited before, that page will be the new head of the linked list.

Also, I have created a struct called a cache where the page information will be stored. The initial time will be set at the beginning of the program and difference with that time for every page, will be the indicator of them.

I am using .txt file to store my blocked addresses which I will not allow users to access.

function add_to_cache:
- Function that I use to add pages to current cache.

function ip_fun:
- Function that I use to check if the hostname is valid. (used gethostbyname to test it)

function find_page_from_cache:
- Finds page from cache by comparing the urls and checking the time.

function host_name_f:
- Parsing the url and getting the hostname

function ip_validation:
- Check if the ip is valid to use. (format wise)

function http_header:
- receives header from server socket

function get_server_response:
- Getting server response to corresponding request

function error_handle:
- Checks errors and checks the blocked.txt function where I'm blocking some addresses.

function client_req:
- Forwarding server response to client

function main:
- Waiting for client input and forking if there's any new client.
