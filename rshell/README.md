# rshell
RSHELL is an open source project created for the CS100 class assignment at the University of California, Riverside.

##How to install
Run the following commands to get the source and build the shell:
```
git clone https://github.com/SparkTesla/rshell.git
cd rshell
make
bin/rshell
```

##Functionality
RSHELL has basic bash command logic with the utilization of combining them with connectors ; && ||

####Example
```
ls || ps || pwd
ps && ls -a ; ls
pwd;pwd;pwd
```

##BUGS
* The problem lies in new processes sprouting after incorrect bash commands resulting in errors.
	You have to exit from these new processes depending on the number of bash command errors.

####Example
```
$ lls
$ ps
$ exit
$ pss
$ exit
$ ps
$ exit
```

##LICENSE
See LICENSE file for details
