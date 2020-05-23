#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "stdtype.h"
#include "midi_funcs.h"
#include "Soundfont.c"
#define cps3_tick (1/(251/4.0))
UINT8 bank_flag[2049] = {0};
UINT16 result_loop_times[16] = {0};
//since the vibrato needs information from the instrument table, i need to make a struct accessible by the make_song function that contains that
struct key_split_struct {
    UINT8 sample_key;
    INT8 detune;
    INT8 min_key;
    INT8 max_key;
}; struct key_split_struct key_split[2048][128];
//tables & converters from vgmtrans
UINT8 rate_convert(INT16 env, UINT8 type, INT16 *new_env){
        //from vgmtrans source code (QsoundInstr.h)
        const uint16_t linear_table[128] = {
        0, 0x3FF, 0x5FE, 0x7FF, 0x9FE, 0xBFE, 0xDFD, 0xFFF, 0x11FE, 0x13FE,
        0x15FD, 0x17FE, 0x19FD, 0x1BFD, 0x1DFC, 0x1FFF, 0x21FD, 0x23FE, 0x25FD,
        0x27FE, 0x29FD, 0x2BFD, 0x2DFC, 0x2FFE, 0x31FD, 0x33FD, 0x35FC, 0x37FD,
        0x39FC, 0x3BFC, 0x3DFB, 0x3FFF, 0x41FE, 0x43FE, 0x45FD, 0x47FE, 0x49FD,
        0x4BFD, 0x4DFC, 0x4FFE, 0x51FD, 0x53FD, 0x55FC, 0x57FD, 0x59FC, 0x5BFC,
        0x5DFB, 0x5FFE, 0x61FD, 0x63FD, 0x65FC, 0x67FD, 0x69FC, 0x6BFC, 0x6DFB,
        0x6FFD, 0x71FC, 0x73FC, 0x75FB, 0x77FC, 0x79FB, 0x7BFB, 0x7DFA, 0x7FFF,
        0x81FE, 0x83FE, 0x85FD, 0x87FE, 0x89FD, 0x8BFD, 0x8DFC, 0x8FFE, 0x91FD,
        0x93FD, 0x95FC, 0x97FD, 0x99FC, 0x9BFC, 0x9DFB, 0x9FFE, 0xA1FD, 0xA3FD,
        0xA5FC, 0xA7FD, 0xA9FC, 0xABFC, 0xADFB, 0xAFFD, 0xB1FC, 0xB3FC,
        0xB5FB, 0xB7FC, 0xB9FB, 0xBBFB, 0xBDFA, 0xBFFE, 0xC1FD, 0xC3FD,
        0xC5FC, 0xC7FD, 0xC9FC, 0xCBFC, 0xCDFB, 0xCFFD, 0xD1FC, 0xD3FC,
        0xD5FB, 0xD7FC, 0xD9FB, 0xDBFB, 0xDDFA, 0xDFFD, 0xE1FC, 0xE3FC,
        0xE5FB, 0xE7FC, 0xE9FB, 0xEBFB, 0xEDFA, 0xEFFC, 0xF1FB, 0xF3FB,
        0xF5FA, 0xF7FB, 0xF9FA, 0xFBFA, 0xFDF9, 0xFFFE
    };
    const uint16_t attack_rate_table[64] = {
        0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7, 8, 9, 0x0B, 0x0D, 0x0F,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x64, 0x84, 0x0A4, 0x0C5, 0x0E6, 0x107, 0x149,
        0x18B, 0x1CC, 0x20E, 0x292, 0x315, 0x398, 0x41C, 0x523, 0x62A, 0x731, 0x838,
        0x0A46, 0x0C54, 0x0E62, 0x1070, 0x148C, 0x18A8, 0x1CC4, 0x20E0, 0x2918,
        0x3150, 0x3989, 0x41C1, 0x5233, 0x62A0, 0x730D, 0x837D, 0xA45D, 0xC54C,
        0xE61C, 0xEFFF, 0xF3FF, 0xF9FF, 0xFCFF, 0xFFFF
    };

    const uint16_t decay_rate_table[64] = {
        0, 1, 2, 2, 3, 3, 4, 4, 5, 6, 8, 0x0A, 0x0C, 0x0E, 0x11, 0x13,
        0x18, 0x1D, 0x21, 0x26, 0x30, 0x39, 0x43, 0x4C, 0x5F, 0x72, 0x85, 0x98, 0x0BE,
        0x0E4, 0x10A, 0x130, 0x17D, 0x1C9, 0x215, 0x260, 0x2F9, 0x391, 0x42A, 0x4C1,
        0x5F2, 0x722, 0x853, 0x983, 0x0BE4, 0x0E46, 0x10A6, 0x1307, 0x17C9, 0x1C8B,
        0x214C, 0x2608, 0x2F92, 0x3916, 0x4299, 0x4C1E, 0x5F24, 0x7228, 0x8533,
        0x9835, 0xBE3E, 0xE451, 0xEFFF, 0xFFFF
    };
    switch (type){
        case 1:{
            *new_env = decay_rate_table[env];
            break;
        }
        case 2:{
            *new_env = attack_rate_table[env];
            break;
        }
        default:{
            *new_env = linear_table[env];
            break;
        }
    }
    return 1;
}
double ConvertPercentAmplitudeToAttenDB_SF2(double percent) {     // assume 0 is -100.0db attenuation
      if (percent == 0)
        return 100.0;
      double atten = 20 * log10(percent);
      return (atten < 100)? -atten: 100.0;
}
double LinAmpDecayTimeToLinDBDecayTime(double secondsToFullAtten, int linearVolumeRange) {
  double expMinDecibel = -100.0;
  double linearMinDecibel = log10(1.0 / linearVolumeRange) * 20.0;
  double linearToExpScale = log(linearMinDecibel - expMinDecibel) / log(2.0);
  return secondsToFullAtten * linearToExpScale;
}
double Log2(double n){
    return log( n ) / log( 2 );
}
double SecondsToTimecents(double secs) {
  INT16 result = 0;
  if (secs < 0.0001) result = -32768;
  else{
    result = log2(secs) * 1200;
  }
  RoundTo16(result);
  return (INT16)result;
}
uint8_t linear2midi(double percent) {
  percent = log(percent / (double)0x7F) / log(2.0) * 6.0;
  return (UINT8)(pow(10.0, percent / 40.0) * 0x7F + 0.5);
}
//structs
struct sample{
    UINT32 start;
    UINT32 end;
    UINT32 size;
    UINT32 loop;
    UINT8 key;
};
struct repeat{
    UINT32 start;
    UINT16 times;
}; struct repeat cps3_repeat[4];
//sample merger
UINT32 merge_sample_roms(UINT8 ** buffer_ptr){

    char filename[64];
    FILE * temp_file = fopen("simm3.0", "rb");
    if (temp_file == NULL) printf("simm file not found");
    int file_pos = 0;
    int pos = 0;
    int file_count = 0;
    int tot_files = 0;
    int tot_size = 0;
    while (temp_file != NULL){
        sprintf(filename, "simm3.%d", tot_files);
        temp_file = fopen(filename, "rb");
        if (temp_file == NULL) break;
        tot_files ++;
        fclose(temp_file);
    }
    FILE * file[tot_files];
    int size[tot_files];
    while (file_count != tot_files){
        sprintf(filename, "simm3.%d", file_count);
        file[file_count] = fopen(filename, "rb");
        if (file[file_count] == NULL) break;
        fseek(file[file_count], 0, SEEK_END);
        size[file_count] = ftell(file[file_count]);
        rewind(file[file_count]);
        tot_size += size[file_count];
        file_count ++;
    }
    file_count = 0;
    printf("\ntot size is %x\n", tot_size);
    *buffer_ptr = malloc(sizeof(UINT8) * tot_size);
    for (file_count = 0; file_count < tot_files; file_count += 2){
        for (file_pos = 0; file_pos < size[file_count]; file_pos ++, pos += 2){
            (*buffer_ptr)[pos] = fgetc(file[file_count + 1]);
            (*buffer_ptr)[pos + 1] = fgetc(file[file_count]);
        }
    }
    for (file_count = 0; file_count < tot_files; file_count ++){
        fclose(file[file_count]);
    }
    return tot_size;
}
//sound font
UINT16 GenerateSampleTable(SF2_DATA* SF2Data, UINT8** RetLoopMsk, UINT8** root_key_array){
    FILE * samplfile = fopen("sample.bin", "rb");
    if (samplfile == NULL){
        printf("error opening sample file");
        exit(EXIT_FAILURE);
    }
    fseek(samplfile, 0, SEEK_END);
    int size = ftell(samplfile);
    printf("the size of sample rom is %x\n", size);
    rewind(samplfile);
    UINT8 * data = malloc(size * sizeof(char));
    fread(data, sizeof(char), size, samplfile);
    fclose(samplfile);
    UINT32 pos = 0x00;
    UINT32 tot_samples = (size / 16);
    sfSample sf2_sample[tot_samples + 2];
    memset(sf2_sample, 0, tot_samples + 2);
    for (int i = 0; i < tot_samples + 2; i++){
        sprintf(sf2_sample[i].achSampleName, "0");
        sf2_sample[i].dwStart = 0;
        sf2_sample[i].dwEnd = 0;
        sf2_sample[i].dwStartloop = 0;
        sf2_sample[i].dwEndloop = 0;
        sf2_sample[i].byOriginalKey = 0;
        sf2_sample[i].dwSampleRate = 0;
        sf2_sample[i].chCorrection = 0;
        sf2_sample[i].sfSampleType = monoSample;
        sf2_sample[i].wSampleLink = 0;
    }
    *RetLoopMsk = (UINT8*)malloc(size);
    memset(*RetLoopMsk, 0, size);
    UINT32 padding = (tot_samples * 46);
    UINT32 num_samples = 0;
    UINT8 * buffer;
    UINT32 samp_data_size = merge_sample_roms(&buffer);
    UINT16* samp_data = malloc((samp_data_size + padding) * sizeof(UINT16));
    memset(samp_data, 0, samp_data_size + padding);
    for (UINT32 x = 0; x < samp_data_size + padding; x++){
        samp_data[x] = 0;
    }
    UINT32 sf2_smp_pos = 0;
    UINT32 sample_rom_pos = 0;
    *root_key_array = malloc(tot_samples * sizeof(UINT8));
	memset(*root_key_array, 0x00, tot_samples);
    for (int x = 0; x < tot_samples; x++){
        (*root_key_array)[x] = 0;
    }
    for (; pos < size; pos += 16, num_samples++){
        padding = (num_samples * 46);
        sprintf(sf2_sample[num_samples].achSampleName, "Sample %d", num_samples);
        UINT32 start = ((data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3]);
        UINT32 loopstart = ((data[pos + 4] << 24) | (data[pos + 5] << 16) | (data[pos + 6] << 8) | data[pos + 7]);
        UINT32 end = ((data[pos + 8] << 24) | (data[pos + 9] << 16) | (data[pos + 10] << 8) | data[pos + 11]);
        INT8 key = data[pos + 15] & 0x7f;
        if (loopstart < end){
            (*RetLoopMsk)[num_samples >> 3] |= 1 << (num_samples & 0x07);
        }
        sf2_sample[num_samples].dwStart = start + padding;
        sf2_sample[num_samples].dwEnd = end + padding;
        sf2_sample[num_samples].dwStartloop = loopstart + padding;
        sf2_sample[num_samples].dwEndloop = end + padding;
        sf2_sample[num_samples].byOriginalKey = key;
        sf2_sample[num_samples].dwSampleRate = 8200;
        (*root_key_array)[num_samples] = key - 26;
        for (; sf2_smp_pos < end + padding; sample_rom_pos ++, sf2_smp_pos++)
            samp_data[sf2_smp_pos] = (INT8)buffer[sample_rom_pos] * 0x100;
        for (int extra_samples_pos = 0; extra_samples_pos < 46; extra_samples_pos ++, sf2_smp_pos++)
            samp_data[sf2_smp_pos] = 0x00;
    }
    sprintf(sf2_sample[tot_samples].achSampleName, "EOS");
    UINT32 SmplDBSize = sizeof(INT16) * (samp_data_size + (num_samples * 46));
	UINT32 SmplHdrSize = sizeof(sfSample) * (tot_samples + 1);
	LIST_CHUNK * LstChk = List_GetChunk(SF2Data->Lists, FCC_sdta);
	ITEM_CHUNK * ItmChk = Item_MakeChunk(FCC_smpl, SmplDBSize, samp_data, 0x00);
	List_AddItem(LstChk, ItmChk);
	LstChk = List_GetChunk(SF2Data->Lists, FCC_pdta);
	ItmChk = Item_MakeChunk(FCC_shdr, SmplHdrSize, sf2_sample, 0x01);	// no free() needed either
	List_AddItem(LstChk, ItmChk);
    return tot_samples;
}
INT8 env_fix(INT16 *atk, INT16 *dec, INT16 *sus, INT16 *srt, INT16 *rel, INT16 *vol){
    //from vgmtrans source code (QsoundInstr.h)
    //i slightly modified it, but i still copied from it
    //because i have been informed that the envelope table found in the cps3 was similar to the one in the cps2
    //actually, im going to "disassemble" the code for the envelopes, and sort-of compare them...
    //not happy with the result i have now, also the mixing is bad...
    UINT16 test = *sus;
    INT8 is_sus_infinite = 0;
    if (test >= 0x7f){
            is_sus_infinite = 1;
    }
    UINT16 temp_pointer[6] = {0};
    rate_convert(*atk, 2, &temp_pointer[0]);
    rate_convert(*dec, 1, &temp_pointer[1]);
    rate_convert(*sus, 0, &temp_pointer[2]);
    rate_convert(*srt, 1, &temp_pointer[3]);
    rate_convert(*rel, 1, &temp_pointer[4]);
    temp_pointer[5] = *vol;
    double ticks = 0;
    double atk_rate = (double)temp_pointer[0];
    double dec_rate = (double)temp_pointer[1];
    double sus_levl = (double)temp_pointer[2];
    double sus_rate = (double)temp_pointer[3];
    double rel_rate = (double)temp_pointer[4];

    double volume = (double)temp_pointer[5];
    //ATTACK ENVELOPE
    ticks = atk_rate ? atk_rate / 0xffff : 0;
    atk_rate = (atk_rate == 0xffff) ? 0 : ticks * cps3_tick;
    //DECAY ENVELOPE
    if (*sus >= 0x7E && *srt > 0 && *dec > 1) { //also, was performing this check with the variables already converted with the table, but it was meant to be used with the unconverted ones
        ticks = (long) ceil((0xFFFF - sus_levl) / (double) dec_rate);
        ticks += (long) ceil(sus_levl / (double) sus_rate);
        dec_rate = ticks * cps3_tick;
        sus_levl = 0.00000001; //original function requests 0 but floats/doubles cant be exacly 0, also log of 0 is undefined
    } else {
        ticks = dec_rate ? (0xFFFF / dec_rate) : 0;
        dec_rate = (dec_rate == 0xFFFF) ? 0 : ticks * cps3_tick;
    }
    dec_rate = LinAmpDecayTimeToLinDBDecayTime(dec_rate, 0x800);
    //SUSTAIN LEVEL (GAIN)
    if (dec_rate <= 1){
        sus_levl = 1;
        is_sus_infinite = 1;
    }else
        sus_levl = sus_levl / (double) 0xFFFF;
    //SUSTAIN RATE (UNSUPPORTED BY SF2)
    ticks = sus_rate ? 0xFFFF / sus_rate : 0;
    sus_rate = (sus_rate == 0xFFFF) ? 0 : ticks * cps3_tick;
    sus_rate = LinAmpDecayTimeToLinDBDecayTime(sus_rate, 0x800);
    //RELEASE
    ticks = rel_rate ? 0xFFFF / rel_rate : 0;
    rel_rate = (rel_rate == 0xFFFF) ? 0 : ticks * cps3_tick;
    rel_rate = LinAmpDecayTimeToLinDBDecayTime(rel_rate, 0x800);
    //btw it was log2() MULTIPLYED by 1200, not divided
    atk_rate = SecondsToTimecents(atk_rate);
    dec_rate = SecondsToTimecents(dec_rate);
    sus_levl = ConvertPercentAmplitudeToAttenDB_SF2(sus_levl);
    sus_levl = (sus_levl >= 100) ? 1000 : 10 * sus_levl;
    sus_rate = SecondsToTimecents(sus_rate);
    rel_rate = SecondsToTimecents(rel_rate);
    volume += 64;
    volume = 0 - (log10(127 / volume) * 200);
    *atk = RoundTo16(atk_rate);
    *dec = RoundTo16(dec_rate);
    *sus = RoundTo16(sus_levl);
    *srt = RoundTo16(sus_rate);
    *rel = RoundTo16(rel_rate);
    *vol = RoundTo16(volume);
    //
    return is_sus_infinite;
}
UINT16 GenerateInstruments(SF2_DATA* SF2Data, UINT16 SmplCnt, const UINT8* LoopMsk, UINT8* root_key_array, UINT8** is_instr_null){
    FILE * instrumentfile = fopen("instrument.bin", "rb");
    if (instrumentfile == NULL){
        printf("error opening instrument file");
        exit(EXIT_FAILURE);
    }
    fseek(instrumentfile, 0, SEEK_END);
    int size = ftell(instrumentfile);
    printf("\tthe size of instrument rom is %x\n", size);
    rewind(instrumentfile);
    UINT8 * data = malloc(size * sizeof(char));
    fread(data, sizeof(char), size, instrumentfile);
    fclose(instrumentfile);
    UINT32 pos = 0;
    UINT32 instr_count = 2048;
    UINT16 bank_count = 16;
    UINT32 bank_offset;
    UINT16 inst_offset;
    UINT32 DataSize;
	sfInst* InsData;
	sfInstBag* InsBags;
	sfInstModList InsMod;
	sfInstGenList* InsGen;
	sfInstGenList* TempGen;
	UINT16 InsBagCnt;
	UINT16 InsBagAlloc;
	UINT16 InsGenCnt;
	UINT16 InsGenAlloc;
	UINT8 LastNote;
	UINT8 CurNote;
	UINT8 MaxNote;
	LIST_CHUNK* LstChk;
	ITEM_CHUNK* ItmChk;
    UINT16 InsAlloc = instr_count + 1;
	InsBagAlloc = InsAlloc * 2;
	InsGenAlloc = InsBagAlloc * 3;
	InsData = (sfInst*)malloc(sizeof(sfInst) * InsAlloc);
	InsBags = (sfInstBag*)malloc(sizeof(sfInstBag) * InsBagAlloc);
	//InsMod = (sfInstModList*)malloc(sizeof(sfInstModList) * 1);
	InsGen = (sfInstGenList*)malloc(sizeof(sfInstGenList) * InsGenAlloc);
	InsBagCnt = 0;
	InsGenCnt = 0;
	UINT16 CurIns = 0;
	*is_instr_null = malloc(InsAlloc);
	memset(*is_instr_null, 0, InsAlloc);
    for (UINT16 i = 0; i < instr_count; i ++){
        (*is_instr_null)[i] = 0;
    }
    UINT32 pointer_address = ((data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3]);
    pointer_address -= 0x40;
    for (pos = 0; pos < 0x40; pos += 4){
        UINT32 bank_offset = ((data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3]);
        //the bank pointers are hard coded, instead of begin an pointer offset
        //this is bad because it would directly go to an unintended address, we need to subtract the actual start address of the bank pointer table
        //the first bank is always located in 0x40 + bank_offset, so we can subtract 0x40 to get that start address

        //btw dont do bank_offset bank_offset -= (bank_offset - 0x40), because its just gonna reset itself to 0
        bank_offset -= pointer_address;
        UINT32 bank_table_pos = pos;
        for (pos = bank_offset; pos < bank_offset + 0xff; pos += 2, CurIns ++){
            UINT16 inst_offset = ((data[pos] << 8) | data[pos + 1]);
            UINT32 bank_pos = pos;
            UINT16 end_instr = 0x0000;
            LastNote = 0;
            if(inst_offset){
            memset(&InsData[CurIns], 0x00, sizeof(sfInst));
            sprintf(InsData[CurIns].achInstName, "Instrument %02hX", CurIns);
            InsData[CurIns].wInstBagNdx = InsBagCnt;
                for(pos = bank_offset + inst_offset; end_instr < 0xffff; pos += 12){
                    CurNote = data[pos];
                    if (CurNote < LastNote) break;
                    if (CurIns == 46){
                        UINT8 check1 = 1;
                    }
                    UINT16 sample_id = (data[pos + 4] << 8) | data[pos + 5];
                    if (sample_id == 0xffff){
                        sample_id = 0;
                        (*is_instr_null)[CurIns] = 1;
                        end_instr = (data[pos + 12] << 8) | data[pos + 13];
                    }
                    else{
                        UINT8 key = root_key_array[sample_id];
                        UINT8 loop_mode; //loop_mode = (LoopMsk[sample_id >> 3] & (1 << (sample_id & 0x07))) ? 1 : 0;
                        if (LoopMsk[sample_id >> 3] & (1 << (sample_id & 0x07)))
                            loop_mode = 0x01;	// Loop on
                        else
                            loop_mode = 0x00; // Loop off
                        INT16 VOL = data[pos + 2];
                        INT16 atk = data[pos + 7];//attack
                        INT16 dec = data[pos + 8];//decay
                        INT16 sus = data[pos + 9];//sustain level
                        INT16 srt = data[pos + 10];//sustain rate
                        INT16 rel = data[pos + 11];//release
                        INT8 is_sus_inf = env_fix(&atk, &dec, &sus, &srt, &rel, &VOL);
                        AddInsBag(&InsBagAlloc, &InsBagCnt, &InsBags, InsGenCnt, 0);
                        AddInsGen_8(&InsGenAlloc, &InsGenCnt, &InsGen, keyRange, LastNote + 1, CurNote);
                        AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, overridingRootKey, key);
                        AddInsGen_U16(&InsGenAlloc, &InsGenCnt, &InsGen, sampleModes, loop_mode);
                        AddInsGen_U16(&InsGenAlloc, &InsGenCnt, &InsGen, sampleID, sample_id);
                        AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, attackVolEnv, atk); //evndata & 0x7fff
                        AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, decayVolEnv, dec);
                        if (!is_sus_inf) AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, sustainVolEnv, sus);
                        AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, releaseVolEnv, rel);
                        //AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, sustainVolEnv, sus);
                        INT8 detune = data[pos + 6];
                        if (detune != 0){
                            UINT8 check1 = 1;
                        }
                        //AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, initialAttenuation, VOL);
                        //AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, fineTune, detune);
                        //AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, velocity, VOL);
                        end_instr = (data[pos + 12] << 8) | data[pos + 13];
                        for (int note = LastNote; note <= CurNote; note ++){
                            key_split[CurIns][note].sample_key = key;
                            key_split[CurIns][note].detune = detune;
                            //printf("\n instrument: %x , note: %x , sample key: %x , vibrato_sensitivity: %x", CurIns, note, key, data[pos + 6]);
                        }
                        LastNote = CurNote;
                    }
                }
            } else{
                memset(&InsData[CurIns], 0x00, sizeof(sfInst));
                sprintf(InsData[CurIns].achInstName, "empty ins.0x%x", CurIns);
                InsData[CurIns].wInstBagNdx = InsBagCnt;
                (*is_instr_null)[CurIns] = 1;
            }
            pos = bank_pos;
        }
        bank_flag[CurIns] = 1;
        pos = bank_table_pos;
    }
    memset(&InsData[CurIns], 0x00, sizeof(sfInst));
    strcpy(InsData[CurIns].achInstName, "EOI");
    InsData[CurIns].wInstBagNdx = InsBagCnt;
    AddInsBag(&InsBagAlloc, &InsBagCnt, &InsBags, InsGenCnt, 0);
    if (InsGenCnt >= InsGenAlloc)
    {
        InsGenAlloc = InsGenCnt + 1;
        InsGen = (sfInstGenList*)realloc(InsGen, sizeof(sfInstGenList) * InsGenAlloc);
    }
    TempGen = &InsGen[InsGenCnt];
    TempGen->sfGenOper = 0; // 'End Of Generators' entry
    TempGen->genAmount.shAmount = 0;
    InsGenCnt ++;
    memset(&InsMod, 0x00, sizeof(sfInstModList));
    LstChk = List_GetChunk(SF2Data->Lists, FCC_pdta);
    DataSize = sizeof(sfInst) * InsAlloc;
    ItmChk = Item_MakeChunk(FCC_inst, DataSize, InsData, 0x00);
    List_AddItem(LstChk, ItmChk);
    DataSize = sizeof(sfInstBag) * InsBagCnt;
    ItmChk = Item_MakeChunk(FCC_ibag, DataSize, InsBags, 0x00);
    List_AddItem(LstChk, ItmChk);
    DataSize = sizeof(sfInstModList);
    ItmChk = Item_MakeChunk(FCC_imod, DataSize, &InsMod, 0x01);
    List_AddItem(LstChk, ItmChk);
    DataSize = sizeof(sfInstGenList) * InsGenCnt;
    ItmChk = Item_MakeChunk(FCC_igen, DataSize, InsGen, 0x00);
    List_AddItem(LstChk, ItmChk);
    return instr_count;
}
static void GeneratePresets(SF2_DATA* SF2Data, UINT16 InsCnt, UINT8* is_instr_null, UINT8 bank_to_copy){
	UINT16 PrsCnt;
	UINT16 PrsAlloc;
	UINT16 CurIns;
	UINT16 CurPrs;
	UINT32 DataSize;
	sfPresetHeader* PrsDB;
	sfPresetHeader* TempPHdr;
	sfPresetBag* PrsBags;
	sfModList PrsMod;
	sfGenList* PrsGen;
	LIST_CHUNK* LstChk;
	ITEM_CHUNK* ItmChk;
	PrsCnt = InsCnt + 128;
	UINT16 bank_lsb = 0;
	PrsAlloc = PrsCnt + 1;
	PrsDB = (sfPresetHeader*)malloc(sizeof(sfPresetHeader) * PrsAlloc);
	PrsBags = (sfPresetBag*)malloc(sizeof(sfPresetBag) * PrsAlloc);
	//PrsMod = (sfModList*)malloc(sizeof(sfModList) * 1);
	PrsGen = (sfGenList*)malloc(sizeof(sfGenList) * PrsAlloc);
	for (CurIns = 0x00, CurPrs = 0x00; CurIns < InsCnt; CurIns ++, CurPrs ++)
	{
        if (bank_flag[CurIns] == 1) bank_lsb ++;
		TempPHdr = &PrsDB[CurPrs];
		memset(TempPHdr, 0x00, sizeof(sfPresetHeader));
		sprintf(TempPHdr->achPresetName, "preset %d", CurIns);
		if (is_instr_null[CurIns] == 1) sprintf(TempPHdr->achPresetName, "empty preset %x", CurIns);
		TempPHdr->wPreset = CurIns & 127;			// MIDI Instrument ID
		TempPHdr->wBank = (UINT16)bank_lsb;			// Bank MSB 0
		TempPHdr->wPresetBagNdx = CurPrs;
		TempPHdr->dwLibrary = 0;			// must be 0
		TempPHdr->dwGenre = 0;				// must be 0
		TempPHdr->dwMorphology = 0;			// must be 0
		PrsBags[CurPrs].wGenNdx = CurPrs;
		PrsBags[CurPrs].wModNdx = 0;
		PrsGen[CurPrs].sfGenOper = instrument;
		PrsGen[CurPrs].genAmount.shAmount = CurIns;
	}
	CurIns = bank_to_copy * 128;
	InsCnt = CurIns + 128;
    for (; CurIns < InsCnt; CurIns ++, CurPrs ++)
	{
		TempPHdr = &PrsDB[CurPrs];
		memset(TempPHdr, 0x00, sizeof(sfPresetHeader));
		sprintf(TempPHdr->achPresetName, "ch10 pres.%d", CurIns);
		if (is_instr_null[CurIns] == 1) sprintf(TempPHdr->achPresetName, "empty preset %x", CurIns);
		TempPHdr->wPreset = CurIns & 127;			// MIDI Instrument ID
		TempPHdr->wBank = 128;			// Bank MSB 0
		TempPHdr->wPresetBagNdx = CurPrs;
		TempPHdr->dwLibrary = 0;			// must be 0
		TempPHdr->dwGenre = 0;				// must be 0
		TempPHdr->dwMorphology = 0;			// must be 0

		PrsBags[CurPrs].wGenNdx = CurPrs;
		PrsBags[CurPrs].wModNdx = 0;
		PrsGen[CurPrs].sfGenOper = instrument;
		PrsGen[CurPrs].genAmount.shAmount = CurIns;
        if (bank_flag[CurIns] == 1) break;
	}
	TempPHdr = &PrsDB[CurPrs];
	memset(TempPHdr, 0x00, sizeof(sfPresetHeader));
	strcpy(TempPHdr->achPresetName, "EOP");	// write "End Of Presets" header
	TempPHdr->wPresetBagNdx = CurPrs;
	PrsBags[CurPrs].wGenNdx = CurPrs;
	PrsBags[CurPrs].wModNdx = 0;
	PrsGen[CurPrs].sfGenOper = 0;		// 'End Of Generators' entry - all 00s
	PrsGen[CurPrs].genAmount.shAmount = 0;
	memset(&PrsMod, 0x00, sizeof(sfModList));
	LstChk = List_GetChunk(SF2Data->Lists, FCC_pdta);
	DataSize = sizeof(sfPresetHeader) * PrsAlloc;
	ItmChk = Item_MakeChunk(FCC_phdr, DataSize, PrsDB, 0x00);
	List_AddItem(LstChk, ItmChk);
	DataSize = sizeof(sfPresetBag) * PrsAlloc;
	ItmChk = Item_MakeChunk(FCC_pbag, DataSize, PrsBags, 0x00);
	List_AddItem(LstChk, ItmChk);
	//DataSize = sizeof(sfModList) * 1;
	DataSize = sizeof(sfModList);
	ItmChk = Item_MakeChunk(FCC_pmod, DataSize, &PrsMod, 0x01);
	List_AddItem(LstChk, ItmChk);
	DataSize = sizeof(sfGenList) * PrsAlloc;
	ItmChk = Item_MakeChunk(FCC_pgen, DataSize, PrsGen, 0x01);
	List_AddItem(LstChk, ItmChk);
	return;
}
static void make_soundfont(const char* FileName, UINT8 bank_to_copy){
    printf("generating soundfont...\n");
    SF2_DATA* SF2Data = 0;
    UINT16 SmplCnt;
	UINT16 InsCnt;
	//test
	UINT8* is_instr_null = 0;
    UINT8* SmplLoopMask = 0;
	UINT8 RetVal;
    UINT8* root_key_array = 0;
	SF2Data = CreateSF2Base("Cps3 soundfont");
    SmplCnt = GenerateSampleTable(SF2Data, &SmplLoopMask, &root_key_array);
	InsCnt = GenerateInstruments(SF2Data, SmplCnt, SmplLoopMask, root_key_array, &is_instr_null);
	GeneratePresets(SF2Data, InsCnt, is_instr_null, bank_to_copy);
	free(SmplLoopMask);
    free(root_key_array);
    free(is_instr_null);
    RetVal = WriteSF2toFile(SF2Data, FileName);
	if (RetVal){
        printf("Save Error: 0x%02X\n", RetVal);
        exit(EXIT_FAILURE);
	}
	FreeSF2Data(SF2Data);
	return;
}
//midi
const UINT8 analyze_song(UINT8* data, UINT32 pos, UINT8 *master_channel, UINT8 sequence){
    UINT32 start = pos;
    UINT32 seq_offset = 0;
    UINT16 chn_offset = 0;
    UINT32 loop_pos[16] = {0};
    UINT32 song_table_pos;
    int temp_chn_count = 0;
    UINT8 type = 0;
    UINT8 loop_start_type = 0;
    UINT16 delay = 0;
    UINT16 note_delay = 0;
    UINT32 tot_delay = 0;
    UINT32 chn_loop_start[16] = {0};
    UINT32 temp_loop_start[4] = {0};
    UINT32 chn_loop_end[16] = {0};
    UINT32 chn_loop_size[16] = {0};
    UINT32 chn_tot_size[16] = {0};
    UINT32 chn_start[16] = {0};
    UINT32 song_lenght = 0;
    UINT32 chn_size = 0;
    UINT8 start_loop_times = 0;
    UINT16 max_chn;
    UINT8 vib_cnt = 0;
    UINT8 check1 = 0;
    INT16 unknow_val = 0;
    seq_offset = ((data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3]);
        for (pos = seq_offset + 1; pos < seq_offset + 33 ; pos += 2){
            chn_offset = ((data[pos] << 8) | data[pos + 1]);
            song_table_pos = pos;
            if (chn_offset != 0x0000){
            //printf("cheching channel %x \t", temp_chn_count);
            //printf("channel offset: %x\n", chn_offset);

            //just in case
            cps3_repeat[0].times = 0;
            cps3_repeat[1].times = 0;
            cps3_repeat[2].times = 0;
            cps3_repeat[3].times = 0;

                for (pos = seq_offset + chn_offset; data[pos] != 0xff; ){
                    if (data[pos] == 0xff) break;
                    if (data[pos] < 0x80){
                        delay = data[pos];
                        while (data[pos + 1] < 0x80){
                            delay = (delay << 7) | (data[pos + 1]);
                            pos ++;
                        }
                        pos ++;
                    }
                else if (data[pos] < 0xc0){
                    if (data[pos + 2] >= 0x80){
                        //note_delay = ((data[pos + 2] & 0x7f ) << 7) | ((data[pos + 3] & 0x7f) << 0); does not increase delays between events.
                        pos += 4;
                    }
                    else{
                        //note_delay = data[pos + 2]; does not increase delays between events.
                        pos += 3;
                    }
                }
                else{
                    switch (data[pos]){
                        case 0xc2:case 0xc3:case 0xc4:case 0xc5:case 0xc6:case 0xc7:case 0xc8:case 0xc9:case 0xdc:case 0xdd:case 0xe0:case 0xe1:case 0xe2:case 0xe6:case 0xe7:{
                            pos += 2;
                            break;
                        }
                        case 0xd8:case 0xd9:case 0xda:case 0xdb:{
                            type = data[pos] - 0xd8;
                            UINT16 skip = ((data[pos + 1] << 8) | data[pos + 2]);
                            if (cps3_repeat[type].times == 1){
                                pos += skip;
                                cps3_repeat[type].times = 0;
                            }
                            pos += 3;
                            break;
                        }
                        case 0xc1:case 0xe8:{
                            pos += 3;
                            break;
                        }
                        case 0xd0:case 0xd1:case 0xd2:case 0xd3:{
                            type = data[pos] - 0xd0;
                            if (cps3_repeat[loop_start_type].times == 0 && type == loop_start_type){
                                chn_loop_start[temp_chn_count] = tot_delay;
                                temp_loop_start[loop_start_type] = chn_loop_start[temp_chn_count];
                            }
                            cps3_repeat[type].start = pos;
                            //printf("loop start is %x ", chn_loop_start[temp_chn_count]);
                            pos ++;
                            break;
                        }
                    case 0xd4:case 0xd5:case 0xd6:case 0xd7:{
                        type = data[pos] - 0xd4;
                            if (type == 1){
                                UINT8 check2 = 1;
                                if (sequence == 0x1a && temp_chn_count == 0){
                                    UINT8 check3 = 1;
                                }
                            }
                            if(cps3_repeat[type].times == 0){
                                cps3_repeat[type].times = data[pos + 1];
                                UINT8 times = data[pos + 1];
                                if (cps3_repeat[type].times == 0 || cps3_repeat[type].times >= 0x7e){
                                        //forgot to put the skip, so here it is.
                                        pos += 2;
                                        if (type != loop_start_type){
                                            chn_loop_start[temp_chn_count] = temp_loop_start[loop_start_type];
                                            loop_start_type = type;
                                            chn_loop_end[temp_chn_count] = tot_delay;
                                            chn_loop_size[temp_chn_count] = (chn_loop_end[temp_chn_count] - chn_loop_start[temp_chn_count]);
                                        }
                                        else if (type == loop_start_type){
                                            chn_loop_end[temp_chn_count] = tot_delay;
                                            chn_loop_size[temp_chn_count] = (chn_loop_end[temp_chn_count] - chn_loop_start[temp_chn_count]);
                                        }
                                    }
                                else pos = cps3_repeat[type].start;
                            }
                            else{
                                cps3_repeat[type].times--;
                                if(cps3_repeat[type].times != 0){
                                    pos = cps3_repeat[type].start;
                                }
                                else{
                                    pos += 2;
                                }
                            }
                        break;
                        }
                        default:{
                            printf("unhandeled command at position %x, the command is %x", pos, data[pos]);
                            exit(EXIT_FAILURE);
                            break;
                        }
                    }
                }
                tot_delay += delay;
                delay = 0;
            }
            chn_tot_size[temp_chn_count] = tot_delay;
            tot_delay = 0;
            temp_chn_count ++;
            if (temp_chn_count == 9){
                UINT8 check2 = 1;
            }
            pos = song_table_pos;
            max_chn = temp_chn_count;
            check1 = 0;
        }
        else temp_chn_count ++;
    }
    song_lenght = 0;
    UINT8 biggest_channel = 0;
    for (temp_chn_count = 0; temp_chn_count < max_chn; temp_chn_count ++){
        if (sequence == 36){
            UINT8 check1 = 1;
        }
        if (song_lenght < chn_loop_size[temp_chn_count]){
            song_lenght = chn_loop_size[temp_chn_count];
            *master_channel = temp_chn_count;
            biggest_channel = temp_chn_count;
        }
    }
    UINT16 min_start = 0xffff;
    for (temp_chn_count = 0; temp_chn_count < max_chn; temp_chn_count ++){
        if (temp_chn_count == 0 && sequence == 26){
            UINT8 check1 = 1;
        }
        if (chn_loop_size[temp_chn_count] == 0) chn_loop_size[temp_chn_count] = song_lenght;
        if (chn_loop_size[temp_chn_count] == song_lenght && min_start > chn_loop_start[temp_chn_count]){
            min_start = chn_loop_start[temp_chn_count];
            *master_channel = temp_chn_count;
        }
    }
    double loop_times;
    //printf("\nsong lenght is %x\n", song_lenght);
    for (temp_chn_count = 0; temp_chn_count < max_chn; temp_chn_count ++){
        if (temp_chn_count == 36 && sequence == 11){
            UINT8 check1 = 1;
        }
        if (chn_loop_size[temp_chn_count] < song_lenght){
            result_loop_times[temp_chn_count] = (((double)song_lenght / chn_loop_size[temp_chn_count]) + 0.5)* 3;
            loop_times = result_loop_times[temp_chn_count];
            // my fault, it rounds to an integer when dividing, if i do the operations separately, or without cast to double, and it can create problems (ex, result is close to 1)
            result_loop_times[temp_chn_count] -= 1;
            UINT32 result_ch_len = (chn_loop_size[temp_chn_count] * loop_times) + chn_loop_start[temp_chn_count];
            UINT32 result_sng_len = (chn_loop_size[*master_channel] * 3) + chn_loop_start[*master_channel];
            INT32 differ = result_sng_len - result_ch_len;
            if (differ > 0){
                result_loop_times[temp_chn_count] += (((double)song_lenght / differ) + 0.5)* 3;
                loop_times = result_loop_times[temp_chn_count];
            }
        }
        else result_loop_times[temp_chn_count] = 2;
       //printf("channel %x's loop times should be %x\n", temp_chn_count, result_loop_times[temp_chn_count]);
    }
    //just in case
    cps3_repeat[0].times = 0;
    cps3_repeat[1].times = 0;
    cps3_repeat[2].times = 0;
    cps3_repeat[3].times = 0;
    pos = start;
    return 1;
}
//process the current lfo value
/*the lfo works like this:
get the vibrato depth and the lfo rate, multiply the 2 to get the lfo value increment / decrement,
if the current lfo state is equal to 1, then the lfo is rising, else its falling
then check whenever the current lfo value is close to the limit, and, if it is:
set the current value to the true lfo limit (shift the value from the vibrato depth table by 16 bits, to get the positive and negative limit)
else, add the lfo increment value (or subtract if the lfo state is 0)*/
const void process_lfo(INT32 *vib_depth, INT32 *lfo_rate, INT8 *lfo_state, INT32 *lfo_limit, INT32 *cur_lfo_val, INT32 *lfo_increment){
    if (*lfo_state == 1){//1 represents a rising lfo, while 0 represents a fallling lfo
        *lfo_limit = *vib_depth - *lfo_increment;//calculate the (close to) maximum limit
        if (*cur_lfo_val >= *lfo_limit){//if the current value is greater that the limit
            *cur_lfo_val = *vib_depth;//set the value of the limit to the lfo value
            *lfo_state = 0;
        }
        else {
            *cur_lfo_val += *lfo_increment;
        }
    }
    else {
        *lfo_limit = *lfo_increment - *vib_depth;//calculate the (close to) minimum limit
        if (*cur_lfo_val <= *lfo_limit){//if the current value is smaller that the limit
            *cur_lfo_val = -(*vib_depth);//set the value of the limit to the lfo value
            *lfo_state = 1;
        }
        else{
            *cur_lfo_val -= *lfo_increment;
        }
    }
}
const void reset_lfo(INT32 *vib_depth, INT32 *lfo_rate, INT8 *lfo_state, INT32 *lfo_limit, INT32 *cur_lfo_val, INT32 *lfo_increment){
    *vib_depth = 0;
    *lfo_state = 1;
    *cur_lfo_val = 0;
    *lfo_increment = 0;
}
const void portamento_fix(double semitone, UINT8 new_rpn){
    INT16 portamento = semitone * 8192 / new_rpn;
    return;
}
const UINT8 make_song(UINT8* data, UINT32 pos, UINT8 master_channel, UINT8 sequence){
    static const UINT16 vol_table[128] = {
        0, 0xA, 0x18, 0x26, 0x34, 0x42, 0x51, 0x5F, 0x6E, 0x7D, 0x8C, 0x9B, 0xAA,
        0xBA, 0xC9, 0xD9, 0xE9, 0xF8, 0x109, 0x119, 0x129, 0x13A, 0x14A, 0x15B,
        0x16C, 0x17D, 0x18E, 0x1A0, 0x1B2, 0x1C3, 0x1D5, 0x1E8, 0x1FC, 0x20D, 0x21F,
        0x232, 0x245, 0x259, 0x26C, 0x280, 0x294, 0x2A8, 0x2BD, 0x2D2, 0x2E7, 0x2FC,
        0x311, 0x327, 0x33D, 0x353, 0x36A, 0x381, 0x398, 0x3B0, 0x3C7, 0x3DF, 0x3F8,
        0x411, 0x42A, 0x443, 0x45D, 0x477, 0x492, 0x4AD, 0x4C8, 0x4E4, 0x501, 0x51D,
        0x53B, 0x558, 0x577, 0x596, 0x5B5, 0x5D5, 0x5F5, 0x616, 0x638, 0x65A, 0x67D,
        0x6A1, 0x6C5, 0x6EB, 0x711, 0x738, 0x75F, 0x788, 0x7B2, 0x7DC, 0x808, 0x834,
        0x862, 0x891, 0x8C2, 0x8F3, 0x927, 0x95B, 0x991, 0x9C9, 0xA03, 0xA3F,
        0xA7D, 0xABD, 0xAFF, 0xB44, 0xB8C, 0xBD7, 0xC25, 0xC76, 0xCCC,
        0xD26, 0xD85, 0xDE9, 0xE53, 0xEC4, 0xF3C, 0xFBD, 0x1048, 0x10DF,
        0x1184, 0x123A, 0x1305, 0x13EA, 0x14F1, 0x1625, 0x179B, 0x1974, 0x1BFB, 0x1FFD
    };
    double msec_tick = 0;
    UINT8 vib_tick_count = 0;
    UINT8 bpm = 0;
    FILE_INF midi_inf;
    MID_TRK_STATE mid_state;
    UINT32 song_offset;
    UINT16 chn_offset;
    UINT32 seq_table_pos = pos;
    UINT32 tempo;
    UINT32 song_loop = 0;
    UINT16 note_lenght = 0;
    UINT16 delay_length = 0;
    UINT8 velocity;
    UINT8 note = 0;
    UINT8 type;
    UINT8 start_type = 0;
    UINT8 tempo_chars[4];
    UINT8 play_mode = 0; //there are multiple playback modes, for example a mode that resets the lfo every new note
    //that can be set by changing the playback enabler (at channel index (at 0x02078d0c + (channel * 0x74)) + 0x62) from 0x20 to any other number less that 0xc0
    //vibrato stuff
    INT32 vib_depth = 0;
    INT32 lfo_rate = 0;
    INT8 lfo_state = 1;
    INT8 vib_on = 0;
    INT32 lfo_limit = 0;
    INT32 cur_lfo_val = 0;
    INT32 lfo_increment = 0;
    double lfo_tick = 0;
    INT32 lfo_percent;
    INT32 key_fraction = 0;
    //these two bytes below have uses inside of the lfo_value to semitone conversion algorithm, that is unknown at the moment
    //always initialized as 0x00 and 0x40
    INT8 byte1 = 0;
    INT8 parameter = 0x40;
    //initialization of the midi file
    midi_inf.alloc = 0x20000;
    midi_inf.data = (UINT8*)malloc(midi_inf.alloc);
    midi_inf.pos = 0x00;
    WriteMidiHeader(&midi_inf, 0x0001, 16, 0x30);
    mid_state.midChn = 0x00;
    UINT16 instr_number = 0;
    UINT8 bank_number = 0;
    INT32 vibrato_value;
    UINT8 check = 0;
    INT8 pitch = 0;
    UINT32 bend_tick = 0;
    UINT8 bend_on = 0;
    INT16 unknow_val = 0;
    UINT32 seq_offset = ((data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3]);
    for (pos = seq_offset + 1; pos < seq_offset + 33 ; pos += 2){
        UINT16 chn_offset = ((data[pos] << 8) | data[pos + 1]);
        if (chn_offset != 0x0000){
            UINT32 tot_tick = 0;
            ////printf("\n in channel %d(offset %x) \n\t", channel, chn_offset + seq_offset);
            mid_state.midChn = mid_state.midChn & 0x0f;
            double cur_lfo_tick = 0;
            WriteMidiTrackStart(&midi_inf, &mid_state);
            WriteEvent(&midi_inf, &mid_state, 0xB0| mid_state.midChn, 0x7E, 00);
            WriteEvent(&midi_inf, &mid_state, 0xB0| mid_state.midChn, 0x7D, 00);
            //in rpn 00, i should be able to adjust the pitch bend range, because right now is too low
            //so, lets set it.
            //the rpn is a set of additional controllers, such as pitch bend range, channel fine and coarse tuning, program and bank tuning...
            WriteEvent(&midi_inf, &mid_state, 0xb0, 100, 00);//controller 100 says that an rpn is now running
            WriteEvent(&midi_inf, &mid_state, 0xb0, 101, 00);//and controller 101 defines what rpn is running, now its running the pitch bend range controller.
            WriteEvent(&midi_inf, &mid_state, 0xb0, 6, 16);//controller 6 gives values to controller 100, and i now giving 16 semitones of range
            WriteEvent(&midi_inf, &mid_state, 0xb0, 100, 0x7f);//this line closes the msb.
            WriteEvent(&midi_inf, &mid_state, 0xb0, 101, 0x7f);//this line closes the lsb.
            UINT32 seq_header_pos = pos;
            UINT8 song_end = 0;
            UINT8 octave = 0;
            UINT8 rpn = 0;
            UINT8 portamento = 0;
            UINT16 note_bend_long = 0x2000;
            INT8 note_bend_final = 0;
            UINT8 note_on = 0;
            INT32 vibrato_final;
            INT32 prev_bend = 0;
            vib_on = 0;
            for (pos = seq_offset + chn_offset; data[pos] != 0xff; ){
                if (sequence == 21 && mid_state.midChn == 0x03){
                    check = 1;
                }
                UINT8 master_loop = 0;
                //printf("pos %x\t", pos);
                    if (data[pos] < 0x80){ //DELAY
                        delay_length = data[pos];
                        while (data[pos + 1] < 0x80){
                            delay_length = (delay_length << 7) | (data[pos + 1]);
                            pos ++;
                        }
                        //printf("delay is now %x", delay_length);
                        pos ++;
                    }
                    else if (data[pos] < 0xc0){ //NOTE ON
                        //1°st byte is the note's velocity, the 2°nd is the note's pitch, 3°rd is the note's duration
                        velocity = (data[pos] - 0x80) * 2;
                        velocity = linear2midi((double) velocity);
                        note = (data[pos + 1]) & 0x7f; //song 10 in jojo has 0xc1 as notes (?)
                        octave = floor((double)note / 12);
                        if(data[pos + 2] >= 0x80) // if the 3°rd byte is bigger that 0x80, the 3°rd and 4°th byte get combined, and become the note's duration
                        {
                            note_lenght = ((data[pos + 2] & 0x7f ) << 7) | ((data[pos + 3] & 0x7f) << 0);
                            pos += 4;
                        }else{
                            note_lenght = data[pos + 2]; //note duration does not increase the delay between events, only tells how many delay ticks the note lasts
                            pos += 3;
                        }
                        WriteEvent(&midi_inf, &mid_state, 0x90, note, velocity);
                        note_on = 1;
                        //printf("midi note %x", note);
                    }
                    else{
                        switch (data[pos]){
                            case 0xc1:{ //TEMPO
                                tempo = ((data[pos + 1] << 8) | data[pos + 2]);
                                tempo = (60000000 / (tempo * 64 / 220)); //not my formula
                                WriteBE32(tempo_chars, tempo);
                                WriteMetaEvent(&midi_inf, &mid_state, 0x51, 0x03, &tempo_chars[1]);
                                msec_tick = tempo / 0x30;
                                //bpm = (60000000 / tempo);
                                //the code for handling the current lfo value is called once per frame
                                //the cps3 refresh rate is 59,583 fps/hz
                                //one frame is 16783.310 microseconds
                                //i need to convert it into ticks
                                //but the timing between tick does change between different tempos
                                //i struggled for a while to find the formula that would convert microseconds into ticks
                                //and found it in a midi utility for phyton named mido
                                //it is a ratio between the microseconds per tick, and the amount of microseconds the frame lasts
                                lfo_tick = 16783.310f / msec_tick;
                                //printf("0xc1 in pos %x", pos);
                                pos += 3;
                                break;
                            }
                            case 0xc2:{ //BANK SELECT
                                //printf("0xc2 in pos %x", pos);
                                bank_number = data[pos + 1];
                                WriteEvent(&midi_inf, &mid_state, 0xb0 | mid_state.midChn, 0, data[pos + 1] & 0x0f);
                                pos += 2;
                                break;
                            }
                            case 0xc3:{ //NOTE BEND
                                //printf("0xc3 in pos %x", mid_state.curDwly);
                                //the bend range is theoretically of 12 semitones, divided into 6144 values (cents), but the midi format uses 8192
                                //6144 / 12 = 512, 8192 / 12 = 682,7 (roughly)
                                //8192 / 512 = 16, so to get 512 cents, you have 16 semitones
                                //portamento works best with 12 semitones, but vibrato works with 16 (maybe)
                                //this is so stupid
                                UINT8 note_bend = data[pos + 1];
                                note_bend_final = note_bend;
                                //WriteEvent(&midi_inf, &mid_state, 0xe0, (note_bend << 6) & 0x40, (note_bend >> 1) & 0x7f);
                                pos += 2;
                                break;
                            }
                            case 0xc4:{ //INSTRUMENT CHANGE
                                //printf("0xc4 in pos %x", pos);
                                instr_number = data[pos + 1];
                                //instr_number += bank_number * 128;
                                WriteEvent(&midi_inf, &mid_state, 0xc0, data[pos + 1] & 0x7f, 0);
                                pos += 2;
                                break;
                            }
                            case 0xc5:{ //VIBRATO (?)
                                //printf("0xc5 in pos %x", pos);
                                vib_depth = data[pos + 1];
                                vib_depth = vibrato_depth_table[vib_depth];
                                //double res_vib_depth = ConvertPercentAmpToStdMidiVal((double) (vibrato_depth_table[vib_depth] * (100 / 256)));
                                vib_on = (vib_depth > 0)? 1: 0;
                                if (vib_on){
                                    lfo_increment = vib_depth * lfo_rate;
                                    vib_depth <<= 16;
                                }
                                else if(play_mode == 1){
                                    reset_lfo(&vib_depth, &lfo_rate, &lfo_state, &lfo_limit, &cur_lfo_val, &lfo_increment);
                                }
                                pos += 2;
                                break;
                            }
                            case 0xc6:{ //CHANNEL VOLUME
                                //printf("0xc6 in pos %x", pos);
                                UINT8 ch_vol1 = data[pos + 1];
                                ch_vol1 = linear2midi(ch_vol1);
                                WriteEvent(&midi_inf, &mid_state, 0xb0, 7, ch_vol1 & 0x7f);
                                pos += 2;
                                break;
                            }

                            case 0xc7:{ //PANNING
                                //printf("0xc7 in pos %x", pos);
                                WriteEvent(&midi_inf, &mid_state, 0xb0, 10, data[pos + 1] & 0x7f);
                                pos += 2;
                                break;
                            }
                            case 0xc8:{ //CHANNEL VOLUME 2
                                //printf("0xc8 in pos %x", pos);
                                UINT8 ch_vol2 = data[pos + 1];
                                ch_vol2 = linear2midi(ch_vol2);
                                WriteEvent(&midi_inf, &mid_state, 0xb0, 11, ch_vol2 & 0x7f);
                                pos += 2;
                                break;
                            }
                            case 0xc9:{ //CHANNEL VOLUME 2
                                pos += 2;
                                break;
                            }
                            case 0xd0:case 0xd1:case 0xd2:case 0xd3:{ //SET REPEAT POINT
                                type = data[pos] - 0xd0; //there are 3 types of repeat, that can be used separately, and can be nested inside an another repeat section
                                //printf("repeat type %x\t", type);
                                cps3_repeat[type].start = pos;
                                //printf("repeat pos %x", cps3_repeat[type].start);
                                pos += 1;
                                break;
                            }
                            case 0xd4:case 0xd5:case 0xd6:case 0xd7:{ //REPEAT FROM REPEAT OF THE SAME TYPE ( ͡q͜ʖ͡q)
                                //printf("repeat in pos %x\tin the midi file, loop point now is %x\t", pos, midi_inf.pos);
                                type = data[pos] - 0xd4; //d4 has type 0, d5 has type 1, d6 has type 2, and d7 has type 3
                                if (sequence == 26 && mid_state.midChn == 0 && data[pos] == 0xd5){
                                    UINT8 check3 = 1;
                                }
                                if(cps3_repeat[type].times == 0){
                                    cps3_repeat[type].times = data[pos + 1]; //reads how many times this repeat command should be processed
                                    UINT16 loops = cps3_repeat[type].times;
                                    //printf("repeat %x begin (times %x)", cps3_repeat[type].start, cps3_repeat[type].times);
                                    if (cps3_repeat[type].times == 0 || cps3_repeat[type].times >= 0x7e){
                                        if (mid_state.midChn == master_channel) {
                                            WriteMetaEvent(&midi_inf, &mid_state, 0x06, strlen("loopStart"), "loopStart");
                                            start_type = type;
                                        }
                                    cps3_repeat[type].times = result_loop_times[mid_state.midChn]; //sometimes, channels have are smaller than others
                                    // so i try to correct them by approximating the correct amount of times in the function check_loop, shown earlier.
                                    master_loop = 1;
                                    //printf("repeat is now %x ", result_loop_times[channel]);
                                    }
                                    pos = cps3_repeat[type].start;
                                }
                                else{ //this section is only for checking, after the command was already processed, if it still needs to be processed again
                                    // only for repeat commands that need to repeat multiple times, even after processing them once
                                    cps3_repeat[type].times--;
                                    if(cps3_repeat[type].times != 0){
                                        pos = cps3_repeat[type].start;
                                    }
                                    else{// if not, then just skip it
                                        pos += 2;
                                        if (mid_state.midChn == master_channel && type == start_type){
                                            WriteMetaEvent(&midi_inf, &mid_state, 0x06, strlen("loopEnd"), "loopEnd");
                                        }
                                    }
                                }
                                break;
                            }
                            case 0xd8:case 0xd9:case 0xda:case 0xdb:{ //JUMP WHEN THE REPEAT SECTION IS FINISHING
                                type = data[pos] - 0xd8;
                                //printf("jump from loop in pos %x", pos);
                                UINT16 skip = ((data[pos + 1] << 8) | data[pos + 2]);
                                if (cps3_repeat[type].times == 1){ //jumps an ammount of bytes (specified by the argument), only when inside of a repeat section that repeats 1 time, or needs to do 1 last time, if it had multiple repeat times
                                    pos += skip;
                                    cps3_repeat[type].times = 0; //then, set repeat times to 0
                                }
                                pos += 3;
                                break;
                            }
                            case 0xdc:{
                                INT8 val = data[pos + 1];
                                unknow_val += (INT16)data;
                                pos += 2;
                                break;
                            }
                            case 0xdd:{
                                INT8 val = data[pos + 1];
                                unknow_val = (INT16)data;
                                pos += 2;
                                break;
                            }
                            case 0xe0:{ //UNSPECIFIED (change playback mode?)
                                //printf("0xe0 in pos %x", pos);
                                play_mode = data[pos + 1];
                                pos += 2;
                                break;
                            }
                            case 0xe1:{ //LFO RATE
                                //printf("0xe1 in pos %x", pos);
                                lfo_rate = data[pos + 1];
                                lfo_rate = lfo_rate_table[lfo_rate];
                                //lfo_spd = lfo_rate_table[lfo_spd] * (100 / 256);
                                pos += 2;
                                break;
                            }
                            case 0xe2:{ //LFO RATE
                                pos += 2;
                                break;
                            }
                            case 0xe6:{ //UNSPECIFIED
                                //printf("0xe7 in pos %x", pos);
                                pos += 2;
                                break;
                            }
                            case 0xe7:{ //UNSPECIFIED
                                //printf("0xe7 in pos %x", pos);
                                pos += 2;
                                break;
                            }
                            case 0xe8:{//MESSAGE TO CPU (OR GPU) (MAINLY USED TO MAKE VISUAL EFFECTS IN ATTRACT INTRO)
                                //printf("0xe8 in pos %x", pos); //none of the messages are supported, i don't know what any of those do, i don't think these are important to the song
                                pos += 3;
                                break;
                            }
                            default:{
                                printf("unhandeled command at position %x, the command is %x", pos, data[pos]);
                                exit(EXIT_FAILURE);
                                break;
                            }
                        }
                    }
                    if(delay_length){ //PROCESS DELAY
                        for(delay_length;  delay_length > 0; delay_length --){
                            if (rpn != 12){
                                rpn = 12;
                                WriteEvent(&midi_inf, &mid_state, 0xb0, 100, 00);
                                WriteEvent(&midi_inf, &mid_state, 0xb0, 101, 00);
                                WriteEvent(&midi_inf, &mid_state, 0xb0, 6, 12);
                                WriteEvent(&midi_inf, &mid_state, 0xb0, 100, 0x7f);
                                WriteEvent(&midi_inf, &mid_state, 0xb0, 101, 0x7f);
                            }
                            /* INT8 sample_key = key_split[instr_number][note].sample_key + 26;
                            INT8 vibrato_sensivity = key_split[instr_number][note].detune;
                            key_fraction = (note - sample_key) + 7;
                            key_fraction = (key_fraction << 8) + 0x80 + vibrato_sensivity;
                            INT32 target_bend = key_fraction; */
                            INT32 bend_final;
                            bend_final = ((note_bend_final) << 6);
                            if (vib_on){
                                vibrato_final = (cur_lfo_val >> 16) << 1;
                                bend_final += (vibrato_final);
                            }
                            if (note_on){
                                INT16 detune = (INT16)key_split[(instr_number) + (bank_number * 128)][note].detune;
                                bend_final += detune << 1;
                            }
                            if (bend_final != prev_bend){
                                prev_bend = bend_final;
                                bend_final += 0x2000;
                                WriteEvent(&midi_inf, &mid_state, 0xe0 | mid_state.midChn, bend_final & 0x7f, (bend_final >> 7) & 0x7f);
                            }
                            //you dont have to reset,  because the game already does so
                            tot_tick ++;
                            mid_state.curDly ++;
                            note_lenght --;
                            if (note_lenght == 0){
                                WriteEvent(&midi_inf, &mid_state ,0x80, note, velocity);
                                note_on = 0;
                            }
                            if (cur_lfo_tick <= tot_tick && vib_on) {
                                process_lfo(&vib_depth, &lfo_rate, &lfo_state, &lfo_limit, &cur_lfo_val, &lfo_increment);

                            }
                            cur_lfo_tick += (cur_lfo_tick <= tot_tick) ? lfo_tick : 0;

                        }
                    }
                //printf("\t song %x\n", sequence);
            }
            WriteEvent(&midi_inf, &mid_state, 0xff, 0x2f, 0x00);
            //printf("channel size is %x", midi_inf.pos);
            WriteMidiTrackEnd(&midi_inf, &mid_state);
            pos = seq_header_pos;
            note_lenght = 0;
            delay_length = 0;
            tot_tick = 0;
            mid_state.midChn ++;
        }
    }
    UINT8 * midi_data = midi_inf.data;
    UINT32 midi_length = midi_inf.pos;
    midi_inf.pos = 0x00;
    WriteMidiHeader(&midi_inf, 0x0001, mid_state.midChn, 0x30);
    write_song(midi_data, midi_length, sequence);
    return 1;
}
const void write_song(UINT8 * data, UINT32 length, int song_id){
    char filename[64];
    sprintf(filename, "song %d.mid", song_id);
    FILE * midi = fopen(filename, "wb");
    if (midi == NULL){
        printf("error opening midi file");
        return 1;
    }
    fwrite(data, 0x01, length, midi);
    fclose(midi);
    return;
}
const void make_music_data(){
    FILE * seqfile = fopen("sequence.bin", "rb");
    if (seqfile == NULL){
        printf("error opening sequence file");
        exit(EXIT_FAILURE);
    }
    fseek(seqfile, 0, SEEK_END);
    int size = ftell(seqfile);
    printf("the size of sequence file is %x\n", size);
    printf("converting midi...\n");
    rewind(seqfile);
    UINT8 * data = malloc(size * sizeof(char));
    fread(data, sizeof(char), size, seqfile);
    fclose(seqfile);
    UINT32 pos = 0x08;
    UINT32 song_offset;
    UINT16 chn_offset;
    int song_id = 0;
    for (pos = 0x08/* pos should always be 0x08 at the start. the following operation is just for debug + 0xb4 */  ; ; pos += 4){
        song_offset = ((data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3]);
        if (song_offset <= 0x00000000){
            break;
        }
        UINT32 seq_table_pos = pos;
        printf("\tconverting sequence %d (pos %x)...", song_id, song_offset);
        UINT8 master_channel = 0;
        //first, pre process song
        if (song_id ==  7){
            UINT8 check = 1;
        }
        analyze_song(data, pos, &master_channel, song_id);
        //then, make the midi
        make_song(data, pos, master_channel, song_id);
        //and, finally, write the song to a new midi file
        printf("done\n");
        pos = seq_table_pos;
        song_id ++; //go to the next song
    }
    free(data);
    return;
}
//main
int main(){
    printf(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n|welcome to the cps3 ripper, the most tedious-to-use music ripper...|\n ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    make_soundfont("song.sf2", 0);
    make_music_data();
    printf("conversion done (this program may make faulty midis)");
    return 0;
}
//end
