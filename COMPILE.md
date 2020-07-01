## How to compile these demos ?
### Install acl rpm into your Linux System
You should make and install rpm of acl first showing below:

#### On CentOS-64 bits
```building
#cd acl/packaging; make
#cd acl/packaging/x86_64; rpm -ivh acl-libs-*.rpm
```

#### On Ubuntu or CentOS-32 bits
```building
#cd acl; make
#make packinstall
```

### Compile these demos
```building demos
#cd demo/c++; make
#cd demo/c++1x; make
```
