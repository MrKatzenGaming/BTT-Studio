## Demangle

char demangledName[0xff];
int status;
size_t bufferLength = sizeof(demangledName);
int* p = nn::init::GetAllocator()->Allocate(bufferLength);
abi::__cxa_demangle(typeid(*<name>).name(), demangledName, &bufferLength, &status);
nn::init::GetAllocator()->free(p);
