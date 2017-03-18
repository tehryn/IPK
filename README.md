#ftrest(1)
##NAME

ftrest - client/server application using RESTful API

##SYNOPSIS

**ftrest** COMMAND [PROTOCOL]HOST[:PORT]/USER/[REMOTE_PATH] [LOCAL_PATH]

**ftrestd** [**-p** *PORT*] [**-r** *ROOT*]

##DESCRIPTION
Application consists of two parts - ftrest and ftrestd. Ftrest represents
client and ftrestd represents server. Client part is used for sending requests
and server part is used for parsing requests and reading/writing files.

##OPTIONS
###COMMAND
Specifies the command that will be used.
* **get** - Copies file from server and store it to file defined by LOCAL_PATH.
If LOCAL_PATH is not set, file will be downloaded to current directory.
* **lst** - Receives list of files and directories in specific directory.
* **del** - Deletes file on server.
* **rmd** - Deletes empty folder on server.
* **mkd** - Creates directory on server.
* **put** - Uploads file to server. LOCAL_PATH must be specified to
use this command.
###PROTOCOL
Specifies the protocol such as http://, tcp://, etc. Default value is http://
###HOST
Specifies host as a name such as localhost, www.example.com, etc
###PORT
Specifies port number that will be used. Default value is 6677
###USER
Specifies user directory. Keep in mind, that you can use only **lst** command
upon this directory
###REMOTE_PATH
Specifies the path to directory or file
###LOCAL_PATH
Specifies the path to local file.
###PORT
Port, where server will bind. Default value is 6677.
###ROOT
Root directory of server. Default directory id current directory.
##DIAGNOSTICS
Following diagnostics might appear on stderr:
* **Not a directory.** - When using mkd, rmd or lst on file.
* **Directory not found.** - When using mkd, rmd or lst but path to directory is invalid.
* **Directory not empty.** - When using rmd on directory that is not empty.
* **Already exists.** - When using mkd but directory already exists (or file with same name).
* **Not a file.** - When using get, del or put but *REMOTE_PATH* leads to directory.
* **File not found.** - When using get, del or put but some file or some of directories in path does not exists.
* **Unknown error** - Unexpected error on server
* **ERROR: *description*** - When you entered invalid arguments such as using put but *LOCAL_PATH* leads to directory,
using mkd or del on *USER*, etc...
##EXAMPLE 1
ftrest mkd http://localhost:12345/xmatej52/new_dir/

Creates directory *new_dir* on server local computer and port number 12345.
##EXAMPLE 2
ftrest put http://localhost:12345/xmatej52/new_file ./hello.txt

Copies file *hello.txt* to local computer into user directory *xmatej52* and
store it as *new_file*.
##EXAMPLE 3
ftrest lst http://localhost:12345/xmatej52/

Prints list of files and folders of user *xmatej52*.
##EXAMPLE 4
ftrest get http://localhost:12345/xmatej52/new_file ./copy.txt

Downloads file *new_file* of user *xmatej52* and store it as *copy.txt*.
##EXAMPLE 5
ftrest del http://localhost:12345/xmatej52/new_file

Deletes file *new_file* of user *xmatej52*.
##EXAMPLE 6
ftrest rmd http://localhost:12345/xmatej52/new_dir/

Removes directory *new_dir* of user *xmatej52*.    
##EXAMPLE 7
ftrestd -p 12345 -r /homes/eva/xm/

Starts server on local computer with binded to port 12345 and rwith root
directory as */homes/eva/xm/*
##BUGS
* Only one client can be connected at time.
* Connecting to *ftrestd* with different client that *ftrest* can cause unexpected behavior

##IMPLEMANTION
###ftrestd
I started with server side of application. First of all, I wrote simple server,
that just sent all requests back as response and then I starting coding client.

###ftrest
I created class *Arguments*, where I wanted to store program settings. I parsed
all parameters, detected hostname, protocol, port number and remote path. I also
checked if there is 'user directory' and if user is not trying to do some operation
with it (mkd, rmd, del, put, get). Then I checked if user is not trying to use command
which works with files on directory (file cann't end with '/'). Then I wrote function
to open a file (when command get was entered) and loaded it into vector. I was afraid
loading binary files into string because of zero character. Since all methods for
working with arguments was finished, I created function for creating http request
and client side was half finished. Because server only echoed what it received, I continued
with server.

###ftrestd
Since request was in correct format, I was able to start parsing it. I created class
Arguments for storing program settings and then created class Request for parsing
Requests from client. I loaded http header, retrieved Content-Length, if there was
and with that information I loaded rest of request. Since I knew type of request
and remote path, I was able to start creating response and completing my task. From
*Linux Programmer's Manual* I read how system functions set *errno* and then detect
errors if there was any. Then only was remaining was response header. After it was done,
I continued working on client side.

###ftrest
I copied Request class from ftrestd and make some little modifications and I was
actually surprised it worked on first try.

##TESTING
I had no time for some advanced testing. I had 2 debuging macros, which I just used
everywhere so I was able to detect where is error quite quickly. Where macros
wern't good enough I used *gdb*. When all was completed, I made some valgrind tests
and then removed all debuging macros.

##CONCLUSION
I had few time for this project, because I didn't expect that 10 points project will
consume such amount of time. To be honest, this code is one of worst I have ever created
and program is not effective as it could be, in fact it is very ineffective and
slow. There are some memory leaks, that I cann't explain. Evan valgrind did not
recognize where they come from. But I also **think** that I did good job and program
works as it should be. It was not easy for me and I am glad we had such projects
in this semester.

##AUTHOR
Written by Matějka Jiří, xmatej52@stud.fit.vutbr.cz
