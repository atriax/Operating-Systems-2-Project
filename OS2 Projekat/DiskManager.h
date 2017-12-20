#ifndef _diskmanager_h_


#include "part.h"

#define _diskmanager_h_

class DiskManager {

public:

	DiskManager(Partition*);
	~DiskManager();

	ClusterNo write(void* content);				// Writes contents onto the partition and returns the number of the cluster they were written on.

	bool hasEnoughSpace(ClusterNo clustersNeeded) { return numberOfFreeClusters >= clustersNeeded; }

private:

	Partition* partition;						// Pointer to the partition.

	ClusterNo* clusterUsageVector;				// Vector of free clusters. Index inside the vector points to the next free cluster number.

	ClusterNo clusterUsageVectorHead = 0;		// Indicates the first next free cluster.
	ClusterNo clusterUsageVectorSize;			// Size of the vector (equal to number of clusters on the partition).
	ClusterNo numberOfFreeClusters;				// Free clusters remaining on the partition.

};


#endif