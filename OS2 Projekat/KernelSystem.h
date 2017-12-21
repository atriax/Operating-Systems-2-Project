#ifndef _kernelsystem_h_

#define _kernelsystem_h_

#include <vector>
#include <iostream>
#include <unordered_map>

#include "vm_declarations.h"
#include "part.h"
#include "DiskManager.h"

class Partition;
class Process;
class KernelProcess;
class KernelSystem;

class KernelSystem {

public:

	KernelSystem(PhysicalAddress processVMSpace, PageNum processVMSpaceSize,
		PhysicalAddress pmtSpace, PageNum pmtSpaceSize,
		Partition* partition);

	~KernelSystem();

	Process* createProcess();
	Time periodicJob();

	// Hardware job
	Status access(ProcessId pid, VirtualAddress address, AccessType type);


private:																		// private attributes

	PhysicalAddress processVMSpace;
	PageNum processVMSpaceSize;

	PhysicalAddress pmtSpace;
	PageNum pmtSpaceSize;

	Partition* partition;

	ProcessId processIDGenerator = 0;											// generates the ID for each new process
	std::unordered_map<ProcessId, Process*> activeProcesses;					// active process hash map

	struct PMT2Descriptor;
	PMT2Descriptor* clockHand;													// clockhand for the second chance swapping algorithm

	struct PMT2DescriptorCounter {
		PhysicalAddress pmt2StartAddress;										// start address of the PMT2
		unsigned short counter = 0;												// number of descriptors in the PMT2 with the inUse bit equal to 1

		PMT2DescriptorCounter(PhysicalAddress startAddress) : pmt2StartAddress(startAddress) {}
	};
	std::unordered_map<unsigned, PMT2DescriptorCounter> activePMT2Counter;		// keeps track of the number of descriptors in the allocated PMT2 tables

	PhysicalAddress freePMTSlotHead;											// head for the PMT1 blocks
	PhysicalAddress freeBlocksHead;												// head for the free physical blocks in memory

	PageNum numberOfFreePMTSlots;												// counts the number of free PMT slots 

	DiskManager* diskManager;													// encapsulates all of the operations with the partition


																				// CONSTANTS


	static const unsigned short usefulBitLength    = 24;
	static const unsigned short page1PartBitLength =  8;						// lengths of parts of the virtual address (in bits)
	static const unsigned short page2PartBitLength =  6;
	static const unsigned short wordPartBitLength  = 10;

	static const unsigned short PMT1Size = 256;									// pmt1 and pmt2 sizes
	static const unsigned short PMT2Size =  64;


																				// MEMORY ORGANISATION


	struct PMT2Descriptor {
		char basicBits = 0;														// _/_/_/execute/write/read/dirty/valid bits
		char advancedBits = 0;													// _/_/_/_/refClockhand/refThrashing/hasCluster/inUse bits

		// bool hasCluster = 0;													// indicates whether a cluster has been reserved for this page
		// bool inUse = 0;														// indicates whether the descriptor is in use yet or not

		PhysicalAddress block = nullptr;										// remember pointer to a block of physical memory
		PMT2Descriptor* next =  nullptr;										// next in segment and next in the global politics swapping technique
		ClusterNo disk;															// which cluster holds this exact page

		PMT2Descriptor() {}
																				// basic bit operations
		void setV() { basicBits |= 0x01; } void resetV() { basicBits &= 0xFE; } bool getV() { return basicBits & 0x01; }
		void setD() { basicBits |= 0x02; } void resetD() { basicBits &= 0xFD; } bool getD() { return basicBits & 0x02; }
		void setRd() { basicBits |= 0x04; } bool getRd() { return basicBits & 0x04; }
		void setWr() { basicBits |= 0x08; } bool getWr() { return basicBits & 0x08; }
		void setRdWr() { basicBits |= 0x0C; } 
		void setEx() { basicBits |= 0x0F; } bool getEx() { return basicBits & 0x0F; }
		
																				// advanced bit operations
		void setRefClockhand() { advancedBits |= 0x08; } void resetRefClockhand() { advancedBits &= 0xF7; }
		bool getRefClockhand() { return advancedBits & 0x08; }

		void setRefThrashing() { advancedBits |= 0x04; } void resetRefThrashing() { advancedBits &= 0xFB; }
		bool getRefThrashing() { return advancedBits & 0x04; }

		void setHasCluster() { advancedBits |= 0x02; } void resetHasCluster() { advancedBits &= 0xFD; }
		bool getHasCluster() { return advancedBits & 0x02; }

		void setInUse() { advancedBits |= 0x01; } void resetInUse() { advancedBits &= 0xFE; }
		bool getInUse() { return advancedBits & 0x01; }

		void setBlock(PhysicalAddress newBlock) { block = newBlock; }
		PhysicalAddress getBlock() { return block; }

		void setDisk(ClusterNo clusterNo) { disk = clusterNo; }
		ClusterNo getDisk() { return disk; }

	};

	typedef PMT2Descriptor PMT2[PMT2Size];
	typedef PMT2* PMT1[PMT1Size];

	friend class Process;
	friend class KernelProcess;

private:

	// DO SYNCHRONISATION

	static PMT2Descriptor* getPageDescriptor(const KernelProcess* process, VirtualAddress address);

	PhysicalAddress getSwappedBlock();											// performs the swapping algorithm and returns a block

	void addDescriptorToClockhandList(PMT2Descriptor*);							// chains a descriptor into the clockhand list

	PhysicalAddress getFreeBlock();												// retrieves a block from the free block list	
	void setFreeBlock(PhysicalAddress block);									// places a now free block to the free block list

	PhysicalAddress getFreePMTSlot();											// retrieves a free PMT1/PMT2 slot (or nullptr if none exist)
	void freePMTSlot(PhysicalAddress slotAddress);								// places a now free PMT1/PMT2 slot to the free slot list

};


#endif