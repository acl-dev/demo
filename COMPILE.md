## How to compile these demos ?
### Install acl rpm into your Linux System
You should make and install rpm of acl first showing below:

#### On CentOS-64 bits
```building
#cd acl/packaging; make
#cd acl/packaging/x86_64; rpm -ivh acl-libs-3.3.0-9.x86_64.rpm
```

#### On Ubuntu or CentOS-32 bits
```building
#cd acl; make
#make packinstall
```

### Compile these demos
```building demos
#cd demo; make
```
