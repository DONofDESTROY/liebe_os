#ifndef STATUS_H
#define STATUS_H

#define OK 0        // everything works fine
#define EIO 1       // io error
#define EINVARG 2   // invlaid argument
#define ENOMEM 3    // No memory
#define EBADPATH 4  // bad file path
#define EFSNOTUS 5  // res indicating that current fs failed to resolve
#define ERDONLY 6   // readonly
#define EUNIMP 7    // un-implemented
#define EISTKN 8    // is taken
#define EINFORMAT 9 // invalid format

#endif // !STATUS_H
