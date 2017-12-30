#ifndef _process_h_

#define _process_h_

#include "vm_declarations.h"

class KernelProcess;
class System;
class KernelSystem;

class Process {

public:

	Process(ProcessId pid);

	~Process();

	ProcessId getProcessId() const;

	Status createSegment(VirtualAddress startAddress, PageNum segmentSize,
		AccessType flags);
	Status loadSegment(VirtualAddress startAddress, PageNum segmentSize,
		AccessType flags, void* content);
	Status deleteSegment(VirtualAddress startAddress);

	Status pageFault(VirtualAddress address);
	PhysicalAddress getPhysicalAddress(VirtualAddress address);

	void blockIfThrashing();

	Process* clone(ProcessId pid);
 	Status createSharedSegment(VirtualAddress startAddress,
 	PageNum segmentSize, const char* name, AccessType flags);
 	Status disconnectSharedSegment(const char* name);
 	Status deleteSharedSegment(const char* name);

private:
	KernelProcess *pProcess;
	friend class System;
	friend class KernelSystem;
	friend class KernelProcess;
};



#endif