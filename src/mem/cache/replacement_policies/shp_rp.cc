/**
 * Copyright (c) 2018-2020 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mem/cache/replacement_policies/shp_rp.hh"

#include <cassert>
#include <memory>

#include "params/SHPRP.hh"
#include "sim/cur_tick.hh"
#include <typeinfo>

/*if (system.l2cache) {
   std::cout << "L2 Cache Associativity: " << system.l2cache->assoc << std::endl;
} else {
    std::cout << "L2 Cache is not configured." << std::endl;
}*/

namespace gem5
{

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{

SHP::SHP(const Params &p)
  : Base(p), associativity(p.associativity)
{
  //assoc = p.associativity;
  //std::cout << "SHP Replacement Policy created with associativity: " << associativity << std::endl;

}

void
SHP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
{
    // Reset last touch timestamp
    std::static_pointer_cast<SHPReplData>(
        replacement_data)->lastTouchTick = Tick(0);
    // Reset insertion tick
    std::static_pointer_cast<SHPReplData>(
        replacement_data)->tickInserted = Tick(0);
    //Make the rank invalid - Setting to -1
    auto replData = std::static_pointer_cast<SHPReplData>(replacement_data);
    for (int i = 0; i < replData->arraySize; i++) {
	replData->dynamicArray[i] = -1;
    }

}

void
SHP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Update last touch timestamp
    std::static_pointer_cast<SHPReplData>(
        replacement_data)->lastTouchTick = curTick();
    auto replData = std::static_pointer_cast<SHPReplData>(replacement_data);
    for (int i = 0; i < replData->arraySize; i++) {
            //std::cout<<"Printing Data outside"<<replData->dynamicArray[i]<<std::endl;
	    if (replData->dynamicArray[i] == -1){
                //std::cout<<"Printing Data Before assigning"<<replData->dynamicArray[i]<<std::endl;
                //Update ranks
                replData->dynamicArray[i] = static_cast<long int>(curTick());
                //std::cout<<"Printing Data After assigning"<<replData->dynamicArray[i]<<std::endl; 
                //std::cout << "Type of dynamicArray: " << typeid(replData->dynamicArray[i]).name() << std::endl;      
	    }
    }
}

void
SHP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Set last touch timestamp
    std::static_pointer_cast<SHPReplData>(
        replacement_data)->lastTouchTick = curTick();
    //Set tick inserted to current timestamp
    std::static_pointer_cast<SHPReplData>(
        replacement_data)->tickInserted = curTick();
    //All the ranks should be zero, except for the rank with itself. (whatever arrayIndex contains should be -1)
    auto replData = std::static_pointer_cast<SHPReplData>(replacement_data);
    int rankIndex = replData->arrayIndex;
    //std::cout<<"Tick is set in reset function: " <<curTick()<<" Inserted tick: "<<std::static_pointer_cast<SHPReplData>(replacement_data)->tickInserted<<std::endl;
    for (int i = 0; i < replData->arraySize; i++) {
        if (i == rankIndex) {
		replData->dynamicArray[i] = -1;
	} else {
		replData->dynamicArray[i] = 0;
	}
    }
}

ReplaceableEntry*
SHP::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    //Printing Candidates Info:
    /*for (size_t i = 0; i < candidates.size(); ++i) {
	std::cout<<"Index: "<<i<<" isMC: "<<std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->isMC<<" tickInserted: "<<std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->tickInserted<<" lastTouchTick: "<<std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->lastTouchTick<<" rankIndex: "<<std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->arrayIndex<<std::endl;
    }*/


    //Initialization
    int numOfSC = 0;
    int desiredSC = static_cast<int>(associativity * 0.5);
    for (const auto& candidate : candidates) {
    	if (std::static_pointer_cast<SHPReplData>(candidate->replacementData)->isMC == false) { numOfSC++; }
    }
	
    if (numOfSC == associativity) {
	//std::cout<<"I'm inside IF statement"<<std::endl;
 	std::vector<std::pair<int, Tick>> indexTickPairs;

	// Populate the index-tick pairs
 	for (size_t i = 0; i < candidates.size(); ++i) {
	    auto tickInserted = std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->tickInserted;
            //std::cout<<"Printing tick inserted: "<<std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->tickInserted<<std::endl;       	   
            indexTickPairs.push_back({i, tickInserted});
        }

        // Simple sort using a loop (ascending order based on tickInserted)
        for (size_t i = 0; i < indexTickPairs.size(); ++i) {
            for (size_t j = i + 1; j < indexTickPairs.size(); ++j) {
                if (indexTickPairs[j].second < indexTickPairs[i].second) {
                    std::swap(indexTickPairs[i], indexTickPairs[j]);
                }
            }
        }

       // Extract sorted indices into a separate array
       std::vector<int> sortArray;
       for (const auto& pair : indexTickPairs) {
           sortArray.push_back(pair.first);
           //std::cout<<"Sort Index: " << pair.first << "Tick Inserted: " << pair.second << std::endl;
       }

       // sortArray now contains the indices of candidates sorted by tickInserted
       
       //Divide MC and SC based on sorted array and give the arrayIndex value accordingly.
       for (int i = 0; i < candidates.size(); i++) {
           if (i < desiredSC) {
	       std::static_pointer_cast<SHPReplData>(candidates[sortArray[i]]->replacementData)->isMC = false;
	       std::static_pointer_cast<SHPReplData>(candidates[sortArray[i]]->replacementData)->arrayIndex = i;
	   } else {
  	       std::static_pointer_cast<SHPReplData>(candidates[sortArray[i]]->replacementData)->isMC = true;
           }
       }

       //Looping through all the candidates and make all array values to invalid(-1) except for the first index in all the cache entries.
       for (size_t i = 0; i < candidates.size(); ++i) {
           for (int j = 1; j < desiredSC; j++) {
	       std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->dynamicArray[j] = -1;
           }
          // std::cout<<"Printing candidate info: isMC: "<<std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->isMC<<" array[0] value: "<<std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->dynamicArray[0]<<std::endl;
       }
    }
 
    //std::cout<<"I'm getting the victim"<<std::endl;

    //Shepherd replacement algorithm for the victim
    //Filter out the actual candidates from the given candidate list. (Filter out MC + oldest SC. tickInserted of SC should be the lowest)
    int filteredCandidatesSize = (associativity - desiredSC + 1);
    int* filteredIndexArray = new int[filteredCandidatesSize];
    int filteredArrayCount = 0;
    int oldSCIndex = -1;
    long int oldSCTick = -1;
    for (size_t i = 0; i < candidates.size(); ++i) {
 	if (std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->isMC == true) {
      	    filteredIndexArray[filteredArrayCount] = i;
	    filteredArrayCount++;
	} else {
	    if (oldSCIndex == -1) {
	    	oldSCIndex = i;
	    	oldSCTick = std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->tickInserted;
	    } else if (std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->tickInserted < oldSCTick) {
	   	oldSCIndex = i;
		oldSCTick = std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->tickInserted;
	    }
	}
    }
    filteredIndexArray[filteredArrayCount] = oldSCIndex;
    filteredArrayCount++;
    
    int oldSCRankIndex = std::static_pointer_cast<SHPReplData>(candidates[oldSCIndex]->replacementData)->arrayIndex; 


    //Printing Candidates Info:
    for (size_t i = 0; i < candidates.size(); ++i) {
            std::cout<<"Index: "<<i<<" SC Rank: "<<std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->dynamicArray[oldSCRankIndex]<<" lastTouchtick: "<<std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->lastTouchTick<<" tickInserted: "<<std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->tickInserted<<std::endl;
    }
                
    std::cout<<"Oldest SC Index: "<<oldSCIndex<<std::endl;
    /*	From the filtered candidates, create another list whose rank is e (-1) wrt oldest SC arrayIndexValue. 
	If this list is not empty, apply LRU on this list. (Lowest lastTouchTick)
	Else, throw out the highest ranked element wrt oldest SC arrayIndexValue.
    */
    
    int victimIndex = -1;
    int invalidIndex = -1;
    long int invalidIndexTick = -1;
    int rankedIndex = -1;
    long int rankedRank = -1;
    
    //std::cout<<"invalidIndex that is set: "<<invalidIndex<<std::endl;
    ReplaceableEntry* victim = candidates[0];
    for (int i = 0; i < filteredArrayCount; i++) {
	//std::cout<<" Rank Value: "<<std::static_pointer_cast<SHPReplData>(candidates[filteredIndexArray[i]]->replacementData)->dynamicArray[oldSCRankIndex]<<std::endl;
	if (std::static_pointer_cast<SHPReplData>(candidates[filteredIndexArray[i]]->replacementData)->dynamicArray[oldSCRankIndex] == -1) {
	    if (invalidIndex == -1 || std::static_pointer_cast<SHPReplData>(candidates[filteredIndexArray[i]]->replacementData)->lastTouchTick < invalidIndexTick) {
		invalidIndex = filteredIndexArray[i];
		invalidIndexTick = std::static_pointer_cast<SHPReplData>(candidates[filteredIndexArray[i]]->replacementData)->lastTouchTick;
		//std::cout<<"I'm inside the IF where rank value is 1 and updating invalidIndex: "<<invalidIndex<<std::endl;
	    }
	} else {
	    if (rankedIndex == -1 || std::static_pointer_cast<SHPReplData>(candidates[filteredIndexArray[i]]->replacementData)->dynamicArray[oldSCRankIndex] > rankedRank) {
		rankedIndex = filteredIndexArray[i];
		rankedRank = std::static_pointer_cast<SHPReplData>(candidates[filteredIndexArray[i]]->replacementData)->dynamicArray[oldSCRankIndex];
	    }
	}	
    }

    if (invalidIndex == -1) {
	victim = candidates[rankedIndex];
        victimIndex = rankedIndex;
    } else {
	victim = candidates[invalidIndex];
        victimIndex = rankedIndex;
    }

    std::cout<<"Victim Index : "<<victimIndex<<std::endl;
    //If (victim is SC) -> do nothing, Else -> oldest SC should become MC.
    if (std::static_pointer_cast<SHPReplData>(victim->replacementData)->isMC == true) {
	std::static_pointer_cast<SHPReplData>(candidates[oldSCIndex]->replacementData)->isMC = true;
    }

    //Make victim as SC
    std::static_pointer_cast<SHPReplData>(victim->replacementData)->isMC = false;

    //Assign oldest SC arrayIndexValue to victim arrayIndexValue
    std::static_pointer_cast<SHPReplData>(victim->replacementData)->arrayIndex = oldSCRankIndex;

    //arrayIndexValue of victim value is found and make that arrayIndex in all candidates to -1.	
    for (size_t i = 0; i < candidates.size(); ++i) {
	std::static_pointer_cast<SHPReplData>(candidates[i]->replacementData)->dynamicArray[oldSCRankIndex] = -1;
    }

    //std::cout<<"Invalid Index: " <<invalidIndex<<" Ranked Index: "<<rankedIndex<<std::endl;
    /* LRU Replacement Policy
    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[0];
    for (const auto& candidate : candidates) {
        // Update victim entry if necessary
        if (std::static_pointer_cast<SHPReplData>(
                    candidate->replacementData)->lastTouchTick <
                std::static_pointer_cast<SHPReplData>(
                    victim->replacementData)->lastTouchTick) {
            victim = candidate;
        }
    }*/

    return victim;
}

std::shared_ptr<ReplacementData>
SHP::instantiateEntry()
{
    int s = static_cast<int>(associativity * 0.5);  //50% of associativity for Shepherd Cache
    //std::cout<<"I'm the instantiation call"<<std::endl;
    return std::shared_ptr<ReplacementData>(new SHPReplData(s));
}

} // namespace replacement_policy
} // namespace gem5
