#include "LCRNG.h"
#include <string>
#include <algorithm>
//#include <iostream>

LCRNG::LCRNG(uint32_t a, uint32_t b, uint32_t aRev, uint32_t bRev, SEED_32 seed)
{
	A = a;
	B = b;
	A_Rev = aRev;
	B_Rev = bRev;
	SeedRNG(seed);
}

LCRNG::LCRNG(uint32_t a, uint32_t b, uint32_t aRev, uint32_t bRev) : LCRNG(a, b, aRev, bRev, 0x0)
{ }

SEED_32 LCRNG::Advance()
{
	seed = (A * seed + B) ;
	return seed;
}

SEED_32 LCRNG::Reverse()
{
	seed = (A_Rev * seed + B_Rev) ;
	return seed;
}

CALL_16 LCRNG::CallRNG()
{
	return this->Advance() >> 16;
}

void LCRNG::SeedRNG(uint32_t seed)
{
	this->seed = seed;
}

SEED_32 LCRNG::GetCurrentSeed()
{
	return seed;
}

void LCRNG::GetIVsAndPID(LCRNG *rng, METHOD method, uint32_t* pid, uint32_t* ivs, std::string* nature)
{
	CALL_16 pidHigh;
	CALL_16 pidLow;
	CALL_16 ivs1 = 0; // Def | Atk | HP
	CALL_16 ivs2 = 0; // SpDef | SpAtk | Spe
	LCRNG tempRNG = *rng;

	// calculate PID (all methods)
	pidLow = tempRNG.CallRNG();
	pidHigh = tempRNG.CallRNG();
	// ivs - advance but skip IV
	if (method == '1')
	{
		ivs1 = tempRNG.CallRNG();
		ivs2 = tempRNG.CallRNG();
	} else if (method == '2')
	{
		tempRNG.CallRNG(); // unused call
		ivs1 = tempRNG.CallRNG();
		ivs2 = tempRNG.CallRNG();
	} else if (method == '4') 
	{  
		ivs1 = tempRNG.CallRNG();
		tempRNG.CallRNG(); // unused call
		ivs2 = tempRNG.CallRNG();
	}
	*pid = (pidHigh << 16) + pidLow;
	*nature = LCRNG::natures[*pid % 25];
	ivs[0] = (ivs1 & IV_HP_MASK) >> 0;
	ivs[1] = (ivs1 & IV_ATK_MASK) >> 5;
	ivs[2] = (ivs1 & IV_DEF_MASK) >> 10;
	ivs[5] = (ivs2 & IV_SPE_MASK) >> 0;
	ivs[3] = (ivs2 & IV_SPA_MASK) >> 5;
	ivs[4] = (ivs2 & IV_SPD_MASK) >> 10;
}

std::vector<SEED_32> LCRNG::SeedsFromPID(PID pid)
{
	std::vector<SEED_32> seeds;
	LCRNG rng(A_32BIT_FWD, B_32BIT_FWD, A_32BIT_REV, B_32BIT_REV);

	//
	CALL_16 call1 = pid & 0xFFFF;
	CALL_16 call2 = pid >> 16;
	SEED_32 seedInit = call1 << 16;
	
	for (int i = 0; i < 0x10000; i++)
	{
		rng.SeedRNG(seedInit + i);
		CALL_16 testCall2 = rng.CallRNG();
		
		if (testCall2 == call2)
		{
            //std::cout << "seed found on iteration " << i << "\n";
			seeds.push_back(rng.Reverse());
		}
	}

	return seeds;
}

std::vector<PID> LCRNG::PIDsFromIVs(uint32_t* ivs)
{
	std::vector<PID> pids;
	CALL_16 ivsCalls1[2] = { 0b0, 0b1000000000000000 }; 
	CALL_16 ivsCalls2[2] = { 0b0, 0b1000000000000000 };

	ivsCalls1[0] = ivsCalls1[0] | ivs[2] << 10 | ivs[1] << 5 | ivs[0]; // 0 |   Def | Atk |  HP
	ivsCalls1[1] = ivsCalls1[1] | ivs[2] << 10 | ivs[1] << 5 | ivs[0]; // 1 |   Def | Atk |  HP
	ivsCalls2[0] = ivsCalls2[0] | ivs[4] << 10 | ivs[3] << 5 | ivs[5]; // 0 | SpDef | SpA | Spe
	ivsCalls2[1] = ivsCalls2[1] | ivs[4] << 10 | ivs[3] << 5 | ivs[5]; // 1 | SpDef | SpA | Spe

	return pids;
}

void LCRNG::RNGStateToGBASeeds(LCRNG *rng, std::vector<SEED_16>* gbaseeds,
                            const std::unordered_set<SEED_16>& filter, std::vector<int>* frames,
                            int maxAdvances) // probably modify to supply number of advances
{
	LCRNG thisrng = *rng;

	for (int i = 0; i < maxAdvances; i++)
	{
		SEED_32 currentSeed = thisrng.GetCurrentSeed();
		if (currentSeed < 0x10000)
		{
            SEED_16 boot = static_cast<SEED_16>(currentSeed);
            if (filter.empty() || filter.count(boot))
            {
                gbaseeds->push_back(boot);
                frames->push_back(i);
            }
		}
		thisrng.Reverse();
	}
}

std::vector<SeedMatch> LCRNG::SeekGBASeeds(const std::vector<SEED_32> &targets,
                                           const std::vector<SEED_16> &filter16,
                                           int maxFrames)
{
    LCRNG rng(A_32BIT_FWD, B_32BIT_FWD, A_32BIT_REV, B_32BIT_REV);
    std::unordered_set<SEED_16> filter(filter16.begin(), filter16.end());
    std::vector<SeedMatch> seedMatches;

    for(SEED_32 target : targets)
    {
        std::vector<SEED_16> hits;
        std::vector<int> hitFrames;
        rng.SeedRNG(target);
        LCRNG::RNGStateToGBASeeds(&rng, &hits, filter, &hitFrames, maxFrames);

        for(int i = 0; i < hits.size(); i++)
        {
            seedMatches.push_back({ target, hits[i], hitFrames[i]});
        }
    }
    return seedMatches;
}

