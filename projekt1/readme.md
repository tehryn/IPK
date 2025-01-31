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

* **get** - Copies file from server and stores it in file defined by LOCAL_PATH.
            If LOCAL_PATH is not set, file will be downloaded to current
            directory.
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
Specifies the path to local file. If *LOCAL_PATH* is directory, file will be
stored into directory.

###PORT
Port to which the server will bind. Default value is 6677.

###ROOT
Root directory of server. Default directory is current directory.

##DIAGNOSTICS
Following diagnostics might appear on stderr:

* **Not a directory.** - When using mkd, rmd or lst on file.
* **Directory not found.** - When using mkd, rmd or lst but path to directory is invalid.
* **Directory not empty.** - When using rmd on directory that is not empty.
* **Already exists.** - When using mkd but directory already exists (or file with same name).
* **Not a file.** - When using get, del or put but *REMOTE_PATH* leads to directory.
* **File not found.** - When using get, del or put but some file or some of directories in path does not exists.
* **Unknown error** - Unexpected error on server
* **ERROR: *description*** - When you entered invalid arguments such as using port that is unable to bind,
                             using mkd or del on *USER*, etc...

##EXAMPLE 1
ftrest mkd http://localhost:12345/xmatej52/new_dir/

Creates directory *new_dir* on server local computer and port number 12345.

##EXAMPLE 2
ftrest put http://localhost:12345/xmatej52/new_file ./hello.txt

Copies file *hello.txt* to local computer into user directory *xmatej52* and
stores it as a *new_file*.

##EXAMPLE 3
ftrest lst http://localhost:12345/xmatej52/

Prints list of files and folders of user *xmatej52*.

##EXAMPLE 4

ftrest get http://localhost:12345/xmatej52/new_file ./copy.txt

Downloads file *new_file* from user *xmatej52* and stores it as *copy.txt*.

##EXAMPLE 5
ftrest del http://localhost:12345/xmatej52/new_file

Deletes file *new_file* from user *xmatej52*.

##EXAMPLE 6
ftrest rmd http://localhost:12345/xmatej52/new_dir/

Removes directory *new_dir* of user *xmatej52*.   

##EXAMPLE 7
ftrestd -p 12345 -r /homes/eva/xm/

Starts server binded to port 12345 on local computer with root directory set to
*/homes/eva/xm/*

##BUGS
* Only one client can be connected at a time.
* Connecting to *ftrestd* with different client than *ftrest* can cause unexpected behavior

##POSSIBLE EXTENSIONS
There was some specified extensions in presentation a some on the forum and sometimes
it wasn't clear what we should do. So I do not know if I made any extensions, but
I will try to explain my "features". It is possible that none of this is extension,
but it will explain the way I wrote program too.

###Protocol
In *ftrest* user is free to specify which protocol he/she wants to use. Default
protocol is set as http, but user is free to enter there any protocol he/she wants.

###Local path
In *ftrest* user does not need to specify local path to file, directory is enough!
Program will retrieve filename from remote path. (only detectable if local path
ends with '/')

###Errors detection before connection
Before *ftrest* tries to connect to host, if user does not do some bad things on
server, such as using *get*, *put* or *del* command on directory (only detectable
if remote path ends with '/'). In this case, client will not connect to host and
write error message on stderr.

###SIGINT and SIGTERM
Since server is running in the infinite loop, *ftrestd* is able to detect SIGTERM and
SIGINT signal and prevent memory leaks.

##AUTHOR
Written by Matějka Jiří, xmatej52@stud.fit.vutbr.cz