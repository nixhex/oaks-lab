#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <array>
#include <unordered_set>

const uint32_t A_32BIT_FWD = 0x41C64E6D;
const uint32_t B_32BIT_FWD = 0x6073;
const uint32_t A_32BIT_REV = 0xEEB9EB65;
const uint32_t B_32BIT_REV = 0x0A3561A1;
const uint32_t IV_DEF_MASK = 0b111110000000000;
const uint32_t IV_ATK_MASK = 0b000001111100000;
const uint32_t IV_HP_MASK = 0b000000000011111;
const uint32_t IV_SPD_MASK = IV_DEF_MASK;
const uint32_t IV_SPA_MASK = IV_ATK_MASK;
const uint32_t IV_SPE_MASK = IV_HP_MASK;

typedef char METHOD;
typedef uint32_t SEED_32;
typedef uint32_t PID;
typedef uint16_t SEED_16;
typedef uint16_t CALL_16;
typedef uint8_t IV_RANGE;
typedef uint8_t GENDER_NUMBER;

typedef struct {
    SEED_32 seed32;
    SEED_16 seed16;
    int frames;
} SeedMatch;

class LCRNG
{
public:
	LCRNG(uint32_t a, uint32_t b, uint32_t aRev, uint32_t bRev, SEED_32 seed);
	LCRNG(uint32_t a, uint32_t b, uint32_t aRev, uint32_t bRev); // seed defaults to 0
	SEED_32 Advance();
	SEED_32 Reverse();
	SEED_32 GetCurrentSeed();
	CALL_16 CallRNG();
	void SeedRNG(uint32_t seed);
	static void GetIVsAndPID(LCRNG* rng, METHOD method, PID *pid, uint32_t *ivs, std::string* nature);
	static std::vector<SEED_32> InitialSeedSeaker(LCRNG rng, METHOD method, std::array<std::array<IV_RANGE,2>,6> IVs, std::string nature, GENDER_NUMBER gender);
	static std::vector<SEED_16> SeedSeeker16(LCRNG rng, METHOD method, uint32_t pid, uint32_t* ivs, std::string* nature);
	static std::vector<SEED_32> SeedsFromPID(PID pid);
	static std::vector<PID> PIDsFromIVs(uint32_t* ivs);
    static void RNGStateToGBASeeds(LCRNG *rng, std::vector<SEED_16>* gbaseeds,
                                   const std::unordered_set<SEED_16>& filters, std::vector<int>* frames,
                                   int maxAdvances);
    static std::vector<SeedMatch> SeekGBASeeds(const std::vector<SEED_32> &targets,
                                               const std::vector<SEED_16> &filter,
                                               int maxFrames);
    static inline const std::string natures[25] = { 
		"Hardy", "Lonely", "Brave", "Adamant", "Naughty",
		"Bold", "Docile", "Relaxed", "Impish", "Lax",
		"Timid", "Hasty", "Serious", "Jolly", "Naive",
		"Modest", "Mild", "Quiet", "Bashful", "Rash",
		"Calm", "Gentle", "Sassy", "Careful", "Quirky" 
	};
protected:
	uint32_t A, A_Rev;
	uint32_t B, B_Rev;
	SEED_32 seed = 0;
};



