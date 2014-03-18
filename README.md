**************************************** SALIENT FEATURES *********************************
* Has option to select the root directory of the server.
* Can handle filenames with more than one word.
* Can use the ls command as if using in your local machine i.e can use the full power of the ls command
* User Interrupts handled.
* Server can handle multiple connections from different clients simultaneously.

### Compiling and Execution
* Open the terminal and cd to the FTP directory
* Run `make` command and it automatically produces the executables of both server and client.
* For producing only server run `make server` and for client run `make client`
* To clean the file run `make clean`
* To start the server run ./server
* To start client run ./client

#### Server Help

$ ./server -h
Usage: server [-h] [-v] [-p <port>] [-r <root directory>]
Options:
-h  This information
-v  Set this option for more verbosity
-p  Port to bind the server to
-r  Root directory of the server

Default root directory: /srv/ftp
Default port: 22

#### Client Help

$ ./client -h
Usage: client [-h] [hostname/IP] [port]
Default hostname/IP is localhost
Default port is 22

### Technical Info

Server and client uses a packet structure as follows

------------------------------------------------------------------------------------------
|TYPE - 1 Byte | MORE FRAGMENTS FLAG - 1 Byte | BYTES TO READ - 2 Bytes | DATA - n Bytes |
------------------------------------------------------------------------------------------

TYPE field is either 0 or 1 corresponding to ERROR type or DATA type.
MORE FRAGMENTS FLAG is either 0 or 1 corresponding to whether this packet is the last or not.
BYTES TO READ field is the number of data bytes in the packet.
DATA field is the actual data. 


### Sample Input/Output

### CLIENT SIDE

  $ ./client 
  Connected to dataport 
  ftp>pwd
  /
  ftp>!pwd
  /home/sanketh/workspace/FTP/Client
  ftp>cd FTP
  Directory change successful
  ftp>pwd
  /FTP/
  ftp>!cd ..
  ftp>!pwd
  /home/sanketh/workspace/FTP
  ftp>ls -l
  total 16
  drwxrwxrwx 2 sanketh sanketh 4096 Mar 17 00:29 Client
  drwxrwxr-x 2 sanketh sanketh 4096 Mar  6 17:54 Docs
  -rw-rw-rw- 1 sanketh sanketh  297 Mar 17 00:27 README
  drwxrwxrwx 2 sanketh sanketh 4096 Mar 17 00:30 Server
  ftp>ls --help
  Usage: ls [OPTION]... [FILE]...
  List information about the FILEs (the current directory by default).
  Sort entries alphabetically if none of -cftuvSUX nor --sort is specified.

  Mandatory arguments to long options are mandatory for short options too.
    -a, --all                  do not ignore entries starting with .
    -A, --almost-all           do not list implied . and ..
        --author               with -l, print the author of each file
    -b, --escape               print C-style escapes for nongraphic characters
        --block-size=SIZE      scale sizes by SIZE before printing them.  E.g.,
                                 '--block-size=M' prints sizes in units of
                                 1,048,576 bytes.  See SIZE format below.
    -B, --ignore-backups       do not list implied entries ending with ~
    -c                         with -lt: sort by, and show, ctime (time of last
                                 modification of file status information)
                                 with -l: show ctime and sort by name
                                 otherwise: sort by ctime, newest first
    -C                         list entries by columns
        --color[=WHEN]         colorize the output.  WHEN defaults to 'always'
                                 or can be 'never' or 'auto'.  More info below
    -d, --directory            list directory entries instead of contents,
                                 and do not dereference symbolic links
    -D, --dired                generate output designed for Emacs' dired mode
    -f                         do not sort, enable -aU, disable -ls --color
    -F, --classify             append indicator (one of */=>@|) to entries
        --file-type            likewise, except do not append '*'
        --format=WORD          across -x, commas -m, horizontal -x, long -l,
                                 single-column -1, verbose -l, vertical -C
        --full-time            like -l --time-style=full-iso
    -g                         like -l, but do not list owner
        --group-directories-first
                               group directories before files.
                                 augment with a --sort option, but any
                                 use of --sort=none (-U) disables grouping
    -G, --no-group             in a long listing, don't print group names
    -h, --human-readable       with -l, print sizes in human readable format
                                 (e.g., 1K 234M 2G)
        --si                   likewise, but use powers of 1000 not 1024
    -H, --dereference-command-line
                               follow symbolic links listed on the command line
        --dereference-command-line-symlink-to-dir
                               follow each command line symbolic link
                               that points to a directory
        --hide=PATTERN         do not list implied entries matching shell PATTERN
                                 (overridden by -a or -A)
        --indicator-style=WORD  append indicator with style WORD to entry names:
                                 none (default), slash (-p),
                                 file-type (--file-type), classify (-F)
    -i, --inode                print the index number of each file
    -I, --ignore=PATTERN       do not list implied entries matching shell PATTERN
    -k, --kibibytes            use 1024-byte blocks
    -l                         use a long listing format
    -L, --dereference          when showing file information for a symbolic
                                 link, show information for the file the link
                                 references rather than for the link itself
    -m                         fill width with a comma separated list of entries
    -n, --numeric-uid-gid      like -l, but list numeric user and group IDs
    -N, --literal              print raw entry names (don't treat e.g. control
                                 characters specially)
    -o                         like -l, but do not list group information
    -p, --indicator-style=slash
                               append / indicator to directories
    -q, --hide-control-chars   print ? instead of non graphic characters
        --show-control-chars   show non graphic characters as-is (default
                               unless program is 'ls' and output is a terminal)
    -Q, --quote-name           enclose entry names in double quotes
        --quoting-style=WORD   use quoting style WORD for entry names:
                                 literal, locale, shell, shell-always, c, escape
    -r, --reverse              reverse order while sorting
    -R, --recursive            list subdirectories recursively
    -s, --size                 print the allocated size of each file, in blocks
    -S                         sort by file size
        --sort=WORD            sort by WORD instead of name: none -U,
                               extension -X, size -S, time -t, version -v
        --time=WORD            with -l, show time as WORD instead of modification
                               time: atime -u, access -u, use -u, ctime -c,
                               or status -c; use specified time as sort key
                               if --sort=time
        --time-style=STYLE     with -l, show times using style STYLE:
                               full-iso, long-iso, iso, locale, +FORMAT.
                               FORMAT is interpreted like 'date'; if FORMAT is
                               FORMAT1<newline>FORMAT2, FORMAT1 applies to
                               non-recent files and FORMAT2 to recent files;
                               if STYLE is prefixed with 'posix-', STYLE
                               takes effect only outside the POSIX locale
    -t                         sort by modification time, newest first
    -T, --tabsize=COLS         assume tab stops at each COLS instead of 8
    -u                         with -lt: sort by, and show, access time
                                 with -l: show access time and sort by name
                                 otherwise: sort by access time
    -U                         do not sort; list entries in directory order
    -v                         natural sort of (version) numbers within text
    -w, --width=COLS           assume screen width instead of current value
    -x                         list entries by lines instead of by columns
    -X                         sort alphabetically by entry extension
    -Z, --context              print any SELinux security context of each file
    -1                         list one file per line
        --help     display this help and exit
        --version  output version information and exit

  SIZE is an integer and optional unit (example: 10M is 10*1024*1024).  Units
  are K, M, G, T, P, E, Z, Y (powers of 1024) or KB, MB, ... (powers of 1000).

  Using color to distinguish file types is disabled both by default and
  with --color=never.  With --color=auto, ls emits color codes only when
  standard output is connected to a terminal.  The LS_COLORS environment
  variable can change the settings.  Use the dircolors command to set it.

  Exit status:
   0  if OK,
   1  if minor problems (e.g., cannot access subdirectory),
   2  if serious trouble (e.g., cannot access command-line argument).

  Report ls bugs to bug-coreutils@gnu.org
  GNU coreutils home page: <http://www.gnu.org/software/coreutils/>
  General help using GNU software: <http://www.gnu.org/gethelp/>
  For complete documentation, run: info coreutils 'ls invocation'
  ftp>ls -z
  ls: invalid option -- 'z'
  Try 'ls --help' for more information.
  ftp>cd /
  Directory change successful
  ftp>
  ftp>!cd Client
  ftp>put client
  ftp>ls client
  client
  ftp>get tftp.h
  OK
  ftp>!ls tftp.h
  tftp.h
  ftp>get FTP
  Is a directory
  ftp>cd something
  No such file or directory
  ftp>cd tftp.h
  Not a directory
  ftp>!ls -z
  ls: invalid option -- 'z'
  Try 'ls --help' for more information.
  ftp>!ls -la
  total 80
  drwxrwxrwx 2 sanketh sanketh  4096 Mar 17 00:29 .
  drwxrwxr-x 6 sanketh sanketh  4096 Mar 17 00:33 ..
  -rwxrwxr-x 1 sanketh sanketh 46668 Mar 16 13:50 client
  -rw-rw-r-- 1 sanketh sanketh  9036 Mar 16 13:51 client.cpp
  -rw-rw-rw- 1 sanketh sanketh  1564 Mar 16 13:45 client.hpp
  -rw-rw-rw- 1 sanketh sanketh   537 Mar 15 22:38 main.cpp
  -rw-rw-r-- 1 sanketh sanketh  1594 Mar 17 00:33 tftp.h
  ftp>get test file
  OK
  ftp>ls "test file"
  test file
  ftp>quit
  Exiting client ...

#### SERVER SIDE

  $ ./server 
  Waiting for connections ... 
  Connection from 127.0.0.1:35396
  Connection to data socket from client port 53339
  PWD successful
  Directory change successful
  PWD successful
  Listing all files and folders
  Listing all files and folders
  Listing all files and folders
  Directory change successful
  client
  Put File Completed
  Listing all files and folders
  File Sent
  Is a directory
  No such file or directory
  Not a directory
  Connection closed
