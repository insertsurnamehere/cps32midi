#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "stdtype.h"
#include "midi_funcs.h"
#include "Soundfont.c"
#define cps3_tick 16783.310
UINT8 bank_flag[2048] = {0};
UINT16 result_loop_times[16] = {0};
//since the vibrato needs information from the instrument table, i need to make a struct accessible by the make_song function that contains that
struct key_split_struct {
    UINT8 sample_key;
    UINT8 vib_sensitivity;
    INT8 min_key;
    INT8 max_key;
}; struct key_split_struct key_split[2048][128];
//tables & converters
UINT16 lfo_value_table[0x1800] = {
    0x0A4C, 0x0A4C, 0x0A4D, 0x0A4D, 0x0A4E, 0x0A4F, 0x0A4F, 0x0A50, 0x0A50,
    0x0A51, 0x0A52, 0x0A52, 0x0A53, 0x0A53, 0x0A54, 0x0A55, 0x0A55, 0x0A56,
    0x0A56, 0x0A57, 0x0A58, 0x0A58, 0x0A59, 0x0A59, 0x0A5A, 0x0A5B, 0x0A5B,
    0x0A5C, 0x0A5C, 0x0A5D, 0x0A5E, 0x0A5E, 0x0A5F, 0x0A5F, 0x0A60, 0x0A60,
    0x0A61, 0x0A62, 0x0A62, 0x0A63, 0x0A63, 0x0A64, 0x0A65, 0x0A65, 0x0A66,
    0x0A67, 0x0A67, 0x0A68, 0x0A68, 0x0A69, 0x0A6A, 0x0A6A, 0x0A6B, 0x0A6B,
    0x0A6C, 0x0A6D, 0x0A6D, 0x0A6E, 0x0A6E, 0x0A6F, 0x0A70, 0x0A70, 0x0A71,
    0x0A71, 0x0A72, 0x0A73, 0x0A73, 0x0A74, 0x0A74, 0x0A75, 0x0A76, 0x0A76,
    0x0A77, 0x0A77, 0x0A78, 0x0A79, 0x0A79, 0x0A7A, 0x0A7A, 0x0A7B, 0x0A7C,
    0x0A7C, 0x0A7D, 0x0A7D, 0x0A7E, 0x0A7F, 0x0A7F, 0x0A80, 0x0A80, 0x0A81,
    0x0A82, 0x0A82, 0x0A83, 0x0A84, 0x0A84, 0x0A85, 0x0A85, 0x0A86, 0x0A87,
    0x0A87, 0x0A88, 0x0A88, 0x0A89, 0x0A8A, 0x0A8A, 0x0A8B, 0x0A8B, 0x0A8C,
    0x0A8D, 0x0A8D, 0x0A8E, 0x0A8E, 0x0A8F, 0x0A90, 0x0A90, 0x0A91, 0x0A92,
    0x0A92, 0x0A93, 0x0A93, 0x0A94, 0x0A95, 0x0A95, 0x0A96, 0x0A96, 0x0A97,
    0x0A98, 0x0A98, 0x0A99, 0x0A99, 0x0A9A, 0x0A9B, 0x0A9B, 0x0A9C, 0x0A9D,
    0x0A9D, 0x0A9E, 0x0A9E, 0x0A9F, 0x0AA0, 0x0AA0, 0x0AA1, 0x0AA1, 0x0AA2,
    0x0AA3, 0x0AA3, 0x0AA4, 0x0AA5, 0x0AA5, 0x0AA6, 0x0AA6, 0x0AA7, 0x0AA8,
    0x0AA8, 0x0AA9, 0x0AA9, 0x0AAA, 0x0AAB, 0x0AAB, 0x0AAC, 0x0AAD, 0x0AAD,
    0x0AAE, 0x0AAE, 0x0AAF, 0x0AB0, 0x0AB0, 0x0AB1, 0x0AB1, 0x0AB2, 0x0AB3,
    0x0AB3, 0x0AB4, 0x0AB5, 0x0AB5, 0x0AB6, 0x0AB6, 0x0AB7, 0x0AB8, 0x0AB8,
    0x0AB9, 0x0ABA, 0x0ABA, 0x0ABB, 0x0ABB, 0x0ABC, 0x0ABD, 0x0ABD, 0x0ABE,
    0x0ABE, 0x0ABF, 0x0AC0, 0x0AC0, 0x0AC1, 0x0AC2, 0x0AC2, 0x0AC3, 0x0AC3,
    0x0AC4, 0x0AC5, 0x0AC5, 0x0AC6, 0x0AC7, 0x0AC7, 0x0AC8, 0x0AC8, 0x0AC9,
    0x0ACA, 0x0ACA, 0x0ACB, 0x0ACC, 0x0ACC, 0x0ACD, 0x0ACD, 0x0ACE, 0x0ACF,
    0x0ACF, 0x0AD0, 0x0AD1, 0x0AD1, 0x0AD2, 0x0AD2, 0x0AD3, 0x0AD4, 0x0AD4,
    0x0AD5, 0x0AD6, 0x0AD6, 0x0AD7, 0x0AD7, 0x0AD8, 0x0AD9, 0x0AD9, 0x0ADA,
    0x0ADB, 0x0ADB, 0x0ADC, 0x0ADC, 0x0ADD, 0x0ADE, 0x0ADE, 0x0ADF, 0x0AE0,
    0x0AE0, 0x0AE1, 0x0AE1, 0x0AE2, 0x0AE3, 0x0AE3, 0x0AE4, 0x0AE5, 0x0AE5,
    0x0AE6, 0x0AE6, 0x0AE7, 0x0AE8, 0x0AE8, 0x0AE9, 0x0AEA, 0x0AEA, 0x0AEB,
    0x0AEC, 0x0AEC, 0x0AED, 0x0AED, 0x0AEE, 0x0AEF, 0x0AEF, 0x0AF0, 0x0AF1,
    0x0AF1, 0x0AF2, 0x0AF2, 0x0AF3, 0x0AF4, 0x0AF4, 0x0AF5, 0x0AF6, 0x0AF6,
    0x0AF7, 0x0AF8, 0x0AF8, 0x0AF9, 0x0AF9, 0x0AFA, 0x0AFB, 0x0AFB, 0x0AFC,
    0x0AFD, 0x0AFD, 0x0AFE, 0x0AFF, 0x0AFF, 0x0B00, 0x0B00, 0x0B01, 0x0B02,
    0x0B02, 0x0B03, 0x0B04, 0x0B04, 0x0B05, 0x0B06, 0x0B06, 0x0B07, 0x0B07,
    0x0B08, 0x0B09, 0x0B09, 0x0B0A, 0x0B0B, 0x0B0B, 0x0B0C, 0x0B0D, 0x0B0D,
    0x0B0E, 0x0B0E, 0x0B0F, 0x0B10, 0x0B10, 0x0B11, 0x0B12, 0x0B12, 0x0B13,
    0x0B14, 0x0B14, 0x0B15, 0x0B15, 0x0B16, 0x0B17, 0x0B17, 0x0B18, 0x0B19,
    0x0B19, 0x0B1A, 0x0B1B, 0x0B1B, 0x0B1C, 0x0B1D, 0x0B1D, 0x0B1E, 0x0B1E,
    0x0B1F, 0x0B20, 0x0B20, 0x0B21, 0x0B22, 0x0B22, 0x0B23, 0x0B24, 0x0B24,
    0x0B25, 0x0B26, 0x0B26, 0x0B27, 0x0B27, 0x0B28, 0x0B29, 0x0B29, 0x0B2A,
    0x0B2B, 0x0B2B, 0x0B2C, 0x0B2D, 0x0B2D, 0x0B2E, 0x0B2F, 0x0B2F, 0x0B30,
    0x0B31, 0x0B31, 0x0B32, 0x0B32, 0x0B33, 0x0B34, 0x0B34, 0x0B35, 0x0B36,
    0x0B36, 0x0B37, 0x0B38, 0x0B38, 0x0B39, 0x0B3A, 0x0B3A, 0x0B3B, 0x0B3C,
    0x0B3C, 0x0B3D, 0x0B3D, 0x0B3E, 0x0B3F, 0x0B3F, 0x0B40, 0x0B41, 0x0B41,
    0x0B42, 0x0B43, 0x0B43, 0x0B44, 0x0B45, 0x0B45, 0x0B46, 0x0B47, 0x0B47,
    0x0B48, 0x0B49, 0x0B49, 0x0B4A, 0x0B4A, 0x0B4B, 0x0B4C, 0x0B4C, 0x0B4D,
    0x0B4E, 0x0B4E, 0x0B4F, 0x0B50, 0x0B50, 0x0B51, 0x0B52, 0x0B52, 0x0B53,
    0x0B54, 0x0B54, 0x0B55, 0x0B56, 0x0B56, 0x0B57, 0x0B58, 0x0B58, 0x0B59,
    0x0B5A, 0x0B5A, 0x0B5B, 0x0B5C, 0x0B5C, 0x0B5D, 0x0B5D, 0x0B5E, 0x0B5F,
    0x0B5F, 0x0B60, 0x0B61, 0x0B61, 0x0B62, 0x0B63, 0x0B63, 0x0B64, 0x0B65,
    0x0B65, 0x0B66, 0x0B67, 0x0B67, 0x0B68, 0x0B69, 0x0B69, 0x0B6A, 0x0B6B,
    0x0B6B, 0x0B6C, 0x0B6D, 0x0B6D, 0x0B6E, 0x0B6F, 0x0B6F, 0x0B70, 0x0B71,
    0x0B71, 0x0B72, 0x0B73, 0x0B73, 0x0B74, 0x0B75, 0x0B75, 0x0B76, 0x0B77,
    0x0B77, 0x0B78, 0x0B79, 0x0B79, 0x0B7A, 0x0B7B, 0x0B7B, 0x0B7C, 0x0B7D,
    0x0B7D, 0x0B7E, 0x0B7E, 0x0B7F, 0x0B80, 0x0B80, 0x0B81, 0x0B82, 0x0B82,
    0x0B83, 0x0B84, 0x0B84, 0x0B85, 0x0B86, 0x0B86, 0x0B87, 0x0B88, 0x0B88,
    0x0B89, 0x0B8A, 0x0B8A, 0x0B8B, 0x0B8C, 0x0B8C, 0x0B8D, 0x0B8E, 0x0B8E,
    0x0B8F, 0x0B90, 0x0B90, 0x0B91, 0x0B92, 0x0B92, 0x0B93, 0x0B94, 0x0B94,
    0x0B95, 0x0B96, 0x0B97, 0x0B97, 0x0B98, 0x0B99, 0x0B99, 0x0B9A, 0x0B9B,
    0x0B9B, 0x0B9C, 0x0B9D, 0x0B9D, 0x0B9E, 0x0B9F, 0x0B9F, 0x0BA0, 0x0BA1,
    0x0BA1, 0x0BA2, 0x0BA3, 0x0BA3, 0x0BA4, 0x0BA5, 0x0BA5, 0x0BA6, 0x0BA7,
    0x0BA7, 0x0BA8, 0x0BA9, 0x0BA9, 0x0BAA, 0x0BAB, 0x0BAB, 0x0BAC, 0x0BAD,
    0x0BAD, 0x0BAE, 0x0BAF, 0x0BAF, 0x0BB0, 0x0BB1, 0x0BB1, 0x0BB2, 0x0BB3,
    0x0BB3, 0x0BB4, 0x0BB5, 0x0BB5, 0x0BB6, 0x0BB7, 0x0BB7, 0x0BB8, 0x0BB9,
    0x0BBA, 0x0BBA, 0x0BBB, 0x0BBC, 0x0BBC, 0x0BBD, 0x0BBE, 0x0BBE, 0x0BBF,
    0x0BC0, 0x0BC0, 0x0BC1, 0x0BC2, 0x0BC2, 0x0BC3, 0x0BC4, 0x0BC4, 0x0BC5,
    0x0BC6, 0x0BC6, 0x0BC7, 0x0BC8, 0x0BC8, 0x0BC9, 0x0BCA, 0x0BCB, 0x0BCB,
    0x0BCC, 0x0BCD, 0x0BCD, 0x0BCE, 0x0BCF, 0x0BCF, 0x0BD0, 0x0BD1, 0x0BD1,
    0x0BD2, 0x0BD3, 0x0BD3, 0x0BD4, 0x0BD5, 0x0BD5, 0x0BD6, 0x0BD7, 0x0BD7,
    0x0BD8, 0x0BD9, 0x0BDA, 0x0BDA, 0x0BDB, 0x0BDC, 0x0BDC, 0x0BDD, 0x0BDE,
    0x0BDE, 0x0BDF, 0x0BE0, 0x0BE0, 0x0BE1, 0x0BE2, 0x0BE2, 0x0BE3, 0x0BE4,
    0x0BE5, 0x0BE5, 0x0BE6, 0x0BE7, 0x0BE7, 0x0BE8, 0x0BE9, 0x0BE9, 0x0BEA,
    0x0BEB, 0x0BEB, 0x0BEC, 0x0BED, 0x0BED, 0x0BEE, 0x0BEF, 0x0BF0, 0x0BF0,
    0x0BF1, 0x0BF2, 0x0BF2, 0x0BF3, 0x0BF4, 0x0BF4, 0x0BF5, 0x0BF6, 0x0BF6,
    0x0BF7, 0x0BF8, 0x0BF9, 0x0BF9, 0x0BFA, 0x0BFB, 0x0BFB, 0x0BFC, 0x0BFD,
    0x0BFD, 0x0BFE, 0x0BFF, 0x0BFF, 0x0C00, 0x0C01, 0x0C02, 0x0C02, 0x0C03,
    0x0C04, 0x0C04, 0x0C05, 0x0C06, 0x0C06, 0x0C07, 0x0C08, 0x0C08, 0x0C09,
    0x0C0A, 0x0C0B, 0x0C0B, 0x0C0C, 0x0C0D, 0x0C0D, 0x0C0E, 0x0C0F, 0x0C0F,
    0x0C10, 0x0C11, 0x0C12, 0x0C12, 0x0C13, 0x0C14, 0x0C14, 0x0C15, 0x0C16,
    0x0C16, 0x0C17, 0x0C18, 0x0C18, 0x0C19, 0x0C1A, 0x0C1B, 0x0C1B, 0x0C1C,
    0x0C1D, 0x0C1D, 0x0C1E, 0x0C1F, 0x0C1F, 0x0C20, 0x0C21, 0x0C22, 0x0C22,
    0x0C23, 0x0C24, 0x0C24, 0x0C25, 0x0C26, 0x0C26, 0x0C27, 0x0C28, 0x0C29,
    0x0C29, 0x0C2A, 0x0C2B, 0x0C2B, 0x0C2C, 0x0C2D, 0x0C2E, 0x0C2E, 0x0C2F,
    0x0C30, 0x0C30, 0x0C31, 0x0C32, 0x0C32, 0x0C33, 0x0C34, 0x0C35, 0x0C35,
    0x0C36, 0x0C37, 0x0C37, 0x0C38, 0x0C39, 0x0C3A, 0x0C3A, 0x0C3B, 0x0C3C,
    0x0C3C, 0x0C3D, 0x0C3E, 0x0C3E, 0x0C3F, 0x0C40, 0x0C41, 0x0C41, 0x0C42,
    0x0C43, 0x0C43, 0x0C44, 0x0C45, 0x0C46, 0x0C46, 0x0C47, 0x0C48, 0x0C48,
    0x0C49, 0x0C4A, 0x0C4B, 0x0C4B, 0x0C4C, 0x0C4D, 0x0C4D, 0x0C4E, 0x0C4F,
    0x0C4F, 0x0C50, 0x0C51, 0x0C52, 0x0C52, 0x0C53, 0x0C54, 0x0C54, 0x0C55,
    0x0C56, 0x0C57, 0x0C57, 0x0C58, 0x0C59, 0x0C59, 0x0C5A, 0x0C5B, 0x0C5C,
    0x0C5C, 0x0C5D, 0x0C5E, 0x0C5E, 0x0C5F, 0x0C60, 0x0C61, 0x0C61, 0x0C62,
    0x0C63, 0x0C63, 0x0C64, 0x0C65, 0x0C66, 0x0C66, 0x0C67, 0x0C68, 0x0C68,
    0x0C69, 0x0C6A, 0x0C6B, 0x0C6B, 0x0C6C, 0x0C6D, 0x0C6D, 0x0C6E, 0x0C6F,
    0x0C70, 0x0C70, 0x0C71, 0x0C72, 0x0C73, 0x0C73, 0x0C74, 0x0C75, 0x0C75,
    0x0C76, 0x0C77, 0x0C78, 0x0C78, 0x0C79, 0x0C7A, 0x0C7A, 0x0C7B, 0x0C7C,
    0x0C7D, 0x0C7D, 0x0C7E, 0x0C7F, 0x0C80, 0x0C80, 0x0C81, 0x0C82, 0x0C82,
    0x0C83, 0x0C84, 0x0C85, 0x0C85, 0x0C86, 0x0C87, 0x0C87, 0x0C88, 0x0C89,
    0x0C8A, 0x0C8A, 0x0C8B, 0x0C8C, 0x0C8D, 0x0C8D, 0x0C8E, 0x0C8F, 0x0C8F,
    0x0C90, 0x0C91, 0x0C92, 0x0C92, 0x0C93, 0x0C94, 0x0C95, 0x0C95, 0x0C96,
    0x0C97, 0x0C97, 0x0C98, 0x0C99, 0x0C9A, 0x0C9A, 0x0C9B, 0x0C9C, 0x0C9D,
    0x0C9D, 0x0C9E, 0x0C9F, 0x0C9F, 0x0CA0, 0x0CA1, 0x0CA2, 0x0CA2, 0x0CA3,
    0x0CA4, 0x0CA5, 0x0CA5, 0x0CA6, 0x0CA7, 0x0CA7, 0x0CA8, 0x0CA9, 0x0CAA,
    0x0CAA, 0x0CAB, 0x0CAC, 0x0CAD, 0x0CAD, 0x0CAE, 0x0CAF, 0x0CB0, 0x0CB0,
    0x0CB1, 0x0CB2, 0x0CB2, 0x0CB3, 0x0CB4, 0x0CB5, 0x0CB5, 0x0CB6, 0x0CB7,
    0x0CB8, 0x0CB8, 0x0CB9, 0x0CBA, 0x0CBB, 0x0CBB, 0x0CBC, 0x0CBD, 0x0CBD,
    0x0CBE, 0x0CBF, 0x0CC0, 0x0CC0, 0x0CC1, 0x0CC2, 0x0CC3, 0x0CC3, 0x0CC4,
    0x0CC5, 0x0CC6, 0x0CC6, 0x0CC7, 0x0CC8, 0x0CC9, 0x0CC9, 0x0CCA, 0x0CCB,
    0x0CCB, 0x0CCC, 0x0CCD, 0x0CCE, 0x0CCE, 0x0CCF, 0x0CD0, 0x0CD1, 0x0CD1,
    0x0CD2, 0x0CD3, 0x0CD4, 0x0CD4, 0x0CD5, 0x0CD6, 0x0CD7, 0x0CD7, 0x0CD8,
    0x0CD9, 0x0CDA, 0x0CDA, 0x0CDB, 0x0CDC, 0x0CDD, 0x0CDD, 0x0CDE, 0x0CDF,
    0x0CE0, 0x0CE0, 0x0CE1, 0x0CE2, 0x0CE2, 0x0CE3, 0x0CE4, 0x0CE5, 0x0CE5,
    0x0CE6, 0x0CE7, 0x0CE8, 0x0CE8, 0x0CE9, 0x0CEA, 0x0CEB, 0x0CEB, 0x0CEC,
    0x0CED, 0x0CEE, 0x0CEE, 0x0CEF, 0x0CF0, 0x0CF1, 0x0CF1, 0x0CF2, 0x0CF3,
    0x0CF4, 0x0CF4, 0x0CF5, 0x0CF6, 0x0CF7, 0x0CF7, 0x0CF8, 0x0CF9, 0x0CFA,
    0x0CFA, 0x0CFB, 0x0CFC, 0x0CFD, 0x0CFD, 0x0CFE, 0x0CFF, 0x0D00, 0x0D00,
    0x0D01, 0x0D02, 0x0D03, 0x0D03, 0x0D04, 0x0D05, 0x0D06, 0x0D06, 0x0D07,
    0x0D08, 0x0D09, 0x0D09, 0x0D0A, 0x0D0B, 0x0D0C, 0x0D0C, 0x0D0D, 0x0D0E,
    0x0D0F, 0x0D0F, 0x0D10, 0x0D11, 0x0D12, 0x0D12, 0x0D13, 0x0D14, 0x0D15,
    0x0D16, 0x0D16, 0x0D17, 0x0D18, 0x0D19, 0x0D19, 0x0D1A, 0x0D1B, 0x0D1C,
    0x0D1C, 0x0D1D, 0x0D1E, 0x0D1F, 0x0D1F, 0x0D20, 0x0D21, 0x0D22, 0x0D22,
    0x0D23, 0x0D24, 0x0D25, 0x0D25, 0x0D26, 0x0D27, 0x0D28, 0x0D28, 0x0D29,
    0x0D2A, 0x0D2B, 0x0D2C, 0x0D2C, 0x0D2D, 0x0D2E, 0x0D2F, 0x0D2F, 0x0D30,
    0x0D31, 0x0D32, 0x0D32, 0x0D33, 0x0D34, 0x0D35, 0x0D35, 0x0D36, 0x0D37,
    0x0D38, 0x0D38, 0x0D39, 0x0D3A, 0x0D3B, 0x0D3C, 0x0D3C, 0x0D3D, 0x0D3E,
    0x0D3F, 0x0D3F, 0x0D40, 0x0D41, 0x0D42, 0x0D42, 0x0D43, 0x0D44, 0x0D45,
    0x0D45, 0x0D46, 0x0D47, 0x0D48, 0x0D49, 0x0D49, 0x0D4A, 0x0D4B, 0x0D4C,
    0x0D4C, 0x0D4D, 0x0D4E, 0x0D4F, 0x0D4F, 0x0D50, 0x0D51, 0x0D52, 0x0D53,
    0x0D53, 0x0D54, 0x0D55, 0x0D56, 0x0D56, 0x0D57, 0x0D58, 0x0D59, 0x0D59,
    0x0D5A, 0x0D5B, 0x0D5C, 0x0D5D, 0x0D5D, 0x0D5E, 0x0D5F, 0x0D60, 0x0D60,
    0x0D61, 0x0D62, 0x0D63, 0x0D64, 0x0D64, 0x0D65, 0x0D66, 0x0D67, 0x0D67,
    0x0D68, 0x0D69, 0x0D6A, 0x0D6A, 0x0D6B, 0x0D6C, 0x0D6D, 0x0D6E, 0x0D6E,
    0x0D6F, 0x0D70, 0x0D71, 0x0D71, 0x0D72, 0x0D73, 0x0D74, 0x0D75, 0x0D75,
    0x0D76, 0x0D77, 0x0D78, 0x0D78, 0x0D79, 0x0D7A, 0x0D7B, 0x0D7C, 0x0D7C,
    0x0D7D, 0x0D7E, 0x0D7F, 0x0D7F, 0x0D80, 0x0D81, 0x0D82, 0x0D83, 0x0D83,
    0x0D84, 0x0D85, 0x0D86, 0x0D86, 0x0D87, 0x0D88, 0x0D89, 0x0D8A, 0x0D8A,
    0x0D8B, 0x0D8C, 0x0D8D, 0x0D8E, 0x0D8E, 0x0D8F, 0x0D90, 0x0D91, 0x0D91,
    0x0D92, 0x0D93, 0x0D94, 0x0D95, 0x0D95, 0x0D96, 0x0D97, 0x0D98, 0x0D99,
    0x0D99, 0x0D9A, 0x0D9B, 0x0D9C, 0x0D9C, 0x0D9D, 0x0D9E, 0x0D9F, 0x0DA0,
    0x0DA0, 0x0DA1, 0x0DA2, 0x0DA3, 0x0DA4, 0x0DA4, 0x0DA5, 0x0DA6, 0x0DA7,
    0x0DA7, 0x0DA8, 0x0DA9, 0x0DAA, 0x0DAB, 0x0DAB, 0x0DAC, 0x0DAD, 0x0DAE,
    0x0DAF, 0x0DAF, 0x0DB0, 0x0DB1, 0x0DB2, 0x0DB3, 0x0DB3, 0x0DB4, 0x0DB5,
    0x0DB6, 0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA, 0x0DBA, 0x0DBB, 0x0DBC,
    0x0DBD, 0x0DBE, 0x0DBE, 0x0DBF, 0x0DC0, 0x0DC1, 0x0DC2, 0x0DC2, 0x0DC3,
    0x0DC4, 0x0DC5, 0x0DC6, 0x0DC6, 0x0DC7, 0x0DC8, 0x0DC9, 0x0DCA, 0x0DCA,
    0x0DCB, 0x0DCC, 0x0DCD, 0x0DCE, 0x0DCE, 0x0DCF, 0x0DD0, 0x0DD1, 0x0DD2,
    0x0DD2, 0x0DD3, 0x0DD4, 0x0DD5, 0x0DD6, 0x0DD6, 0x0DD7, 0x0DD8, 0x0DD9,
    0x0DDA, 0x0DDA, 0x0DDB, 0x0DDC, 0x0DDD, 0x0DDE, 0x0DDE, 0x0DDF, 0x0DE0,
    0x0DE1, 0x0DE2, 0x0DE2, 0x0DE3, 0x0DE4, 0x0DE5, 0x0DE6, 0x0DE6, 0x0DE7,
    0x0DE8, 0x0DE9, 0x0DEA, 0x0DEA, 0x0DEB, 0x0DEC, 0x0DED, 0x0DEE, 0x0DEE,
    0x0DEF, 0x0DF0, 0x0DF1, 0x0DF2, 0x0DF2, 0x0DF3, 0x0DF4, 0x0DF5, 0x0DF6,
    0x0DF6, 0x0DF7, 0x0DF8, 0x0DF9, 0x0DFA, 0x0DFA, 0x0DFB, 0x0DFC, 0x0DFD,
    0x0DFE, 0x0DFF, 0x0DFF, 0x0E00, 0x0E01, 0x0E02, 0x0E03, 0x0E03, 0x0E04,
    0x0E05, 0x0E06, 0x0E07, 0x0E07, 0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0B,
    0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F, 0x0E10, 0x0E10, 0x0E11, 0x0E12, 0x0E13,
    0x0E14, 0x0E14, 0x0E15, 0x0E16, 0x0E17, 0x0E18, 0x0E19, 0x0E19, 0x0E1A,
    0x0E1B, 0x0E1C, 0x0E1D, 0x0E1D, 0x0E1E, 0x0E1F, 0x0E20, 0x0E21, 0x0E21,
    0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E26, 0x0E27, 0x0E28, 0x0E29,
    0x0E2A, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E, 0x0E2F, 0x0E2F, 0x0E30,
    0x0E31, 0x0E32, 0x0E33, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37, 0x0E38,
    0x0E38, 0x0E39, 0x0E3A, 0x0E3B, 0x0E3C, 0x0E3D, 0x0E3D, 0x0E3E, 0x0E3F,
    0x0E40, 0x0E41, 0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E46,
    0x0E47, 0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E,
    0x0E4F, 0x0E4F, 0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E54, 0x0E55,
    0x0E56, 0x0E57, 0x0E58, 0x0E59, 0x0E59, 0x0E5A, 0x0E5B, 0x0E5C, 0x0E5D,
    0x0E5E, 0x0E5E, 0x0E5F, 0x0E60, 0x0E61, 0x0E62, 0x0E63, 0x0E63, 0x0E64,
    0x0E65, 0x0E66, 0x0E67, 0x0E68, 0x0E68, 0x0E69, 0x0E6A, 0x0E6B, 0x0E6C,
    0x0E6D, 0x0E6D, 0x0E6E, 0x0E6F, 0x0E70, 0x0E71, 0x0E72, 0x0E72, 0x0E73,
    0x0E74, 0x0E75, 0x0E76, 0x0E77, 0x0E77, 0x0E78, 0x0E79, 0x0E7A, 0x0E7B,
    0x0E7C, 0x0E7C, 0x0E7D, 0x0E7E, 0x0E7F, 0x0E80, 0x0E81, 0x0E81, 0x0E82,
    0x0E83, 0x0E84, 0x0E85, 0x0E86, 0x0E86, 0x0E87, 0x0E88, 0x0E89, 0x0E8A,
    0x0E8B, 0x0E8C, 0x0E8C, 0x0E8D, 0x0E8E, 0x0E8F, 0x0E90, 0x0E91, 0x0E91,
    0x0E92, 0x0E93, 0x0E94, 0x0E95, 0x0E96, 0x0E96, 0x0E97, 0x0E98, 0x0E99,
    0x0E9A, 0x0E9B, 0x0E9C, 0x0E9C, 0x0E9D, 0x0E9E, 0x0E9F, 0x0EA0, 0x0EA1,
    0x0EA1, 0x0EA2, 0x0EA3, 0x0EA4, 0x0EA5, 0x0EA6, 0x0EA6, 0x0EA7, 0x0EA8,
    0x0EA9, 0x0EAA, 0x0EAB, 0x0EAC, 0x0EAC, 0x0EAD, 0x0EAE, 0x0EAF, 0x0EB0,
    0x0EB1, 0x0EB2, 0x0EB2, 0x0EB3, 0x0EB4, 0x0EB5, 0x0EB6, 0x0EB7, 0x0EB7,
    0x0EB8, 0x0EB9, 0x0EBA, 0x0EBB, 0x0EBC, 0x0EBD, 0x0EBD, 0x0EBE, 0x0EBF,
    0x0EC0, 0x0EC1, 0x0EC2, 0x0EC3, 0x0EC3, 0x0EC4, 0x0EC5, 0x0EC6, 0x0EC7,
    0x0EC8, 0x0EC9, 0x0EC9, 0x0ECA, 0x0ECB, 0x0ECC, 0x0ECD, 0x0ECE, 0x0ECE,
    0x0ECF, 0x0ED0, 0x0ED1, 0x0ED2, 0x0ED3, 0x0ED4, 0x0ED4, 0x0ED5, 0x0ED6,
    0x0ED7, 0x0ED8, 0x0ED9, 0x0EDA, 0x0EDA, 0x0EDB, 0x0EDC, 0x0EDD, 0x0EDE,
    0x0EDF, 0x0EE0, 0x0EE0, 0x0EE1, 0x0EE2, 0x0EE3, 0x0EE4, 0x0EE5, 0x0EE6,
    0x0EE7, 0x0EE7, 0x0EE8, 0x0EE9, 0x0EEA, 0x0EEB, 0x0EEC, 0x0EED, 0x0EED,
    0x0EEE, 0x0EEF, 0x0EF0, 0x0EF1, 0x0EF2, 0x0EF3, 0x0EF3, 0x0EF4, 0x0EF5,
    0x0EF6, 0x0EF7, 0x0EF8, 0x0EF9, 0x0EFA, 0x0EFA, 0x0EFB, 0x0EFC, 0x0EFD,
    0x0EFE, 0x0EFF, 0x0F00, 0x0F00, 0x0F01, 0x0F02, 0x0F03, 0x0F04, 0x0F05,
    0x0F06, 0x0F07, 0x0F07, 0x0F08, 0x0F09, 0x0F0A, 0x0F0B, 0x0F0C, 0x0F0D,
    0x0F0D, 0x0F0E, 0x0F0F, 0x0F10, 0x0F11, 0x0F12, 0x0F13, 0x0F14, 0x0F14,
    0x0F15, 0x0F16, 0x0F17, 0x0F18, 0x0F19, 0x0F1A, 0x0F1B, 0x0F1B, 0x0F1C,
    0x0F1D, 0x0F1E, 0x0F1F, 0x0F20, 0x0F21, 0x0F22, 0x0F22, 0x0F23, 0x0F24,
    0x0F25, 0x0F26, 0x0F27, 0x0F28, 0x0F29, 0x0F29, 0x0F2A, 0x0F2B, 0x0F2C,
    0x0F2D, 0x0F2E, 0x0F2F, 0x0F30, 0x0F30, 0x0F31, 0x0F32, 0x0F33, 0x0F34,
    0x0F35, 0x0F36, 0x0F37, 0x0F37, 0x0F38, 0x0F39, 0x0F3A, 0x0F3B, 0x0F3C,
    0x0F3D, 0x0F3E, 0x0F3E, 0x0F3F, 0x0F40, 0x0F41, 0x0F42, 0x0F43, 0x0F44,
    0x0F45, 0x0F46, 0x0F46, 0x0F47, 0x0F48, 0x0F49, 0x0F4A, 0x0F4B, 0x0F4C,
    0x0F4D, 0x0F4D, 0x0F4E, 0x0F4F, 0x0F50, 0x0F51, 0x0F52, 0x0F53, 0x0F54,
    0x0F55, 0x0F55, 0x0F56, 0x0F57, 0x0F58, 0x0F59, 0x0F5A, 0x0F5B, 0x0F5C,
    0x0F5D, 0x0F5D, 0x0F5E, 0x0F5F, 0x0F60, 0x0F61, 0x0F62, 0x0F63, 0x0F64,
    0x0F65, 0x0F65, 0x0F66, 0x0F67, 0x0F68, 0x0F69, 0x0F6A, 0x0F6B, 0x0F6C,
    0x0F6D, 0x0F6D, 0x0F6E, 0x0F6F, 0x0F70, 0x0F71, 0x0F72, 0x0F73, 0x0F74,
    0x0F75, 0x0F75, 0x0F76, 0x0F77, 0x0F78, 0x0F79, 0x0F7A, 0x0F7B, 0x0F7C,
    0x0F7D, 0x0F7E, 0x0F7E, 0x0F7F, 0x0F80, 0x0F81, 0x0F82, 0x0F83, 0x0F84,
    0x0F85, 0x0F86, 0x0F86, 0x0F87, 0x0F88, 0x0F89, 0x0F8A, 0x0F8B, 0x0F8C,
    0x0F8D, 0x0F8E, 0x0F8F, 0x0F8F, 0x0F90, 0x0F91, 0x0F92, 0x0F93, 0x0F94,
    0x0F95, 0x0F96, 0x0F97, 0x0F98, 0x0F98, 0x0F99, 0x0F9A, 0x0F9B, 0x0F9C,
    0x0F9D, 0x0F9E, 0x0F9F, 0x0FA0, 0x0FA1, 0x0FA1, 0x0FA2, 0x0FA3, 0x0FA4,
    0x0FA5, 0x0FA6, 0x0FA7, 0x0FA8, 0x0FA9, 0x0FAA, 0x0FAB, 0x0FAB, 0x0FAC,
    0x0FAD, 0x0FAE, 0x0FAF, 0x0FB0, 0x0FB1, 0x0FB2, 0x0FB3, 0x0FB4, 0x0FB4,
    0x0FB5, 0x0FB6, 0x0FB7, 0x0FB8, 0x0FB9, 0x0FBA, 0x0FBB, 0x0FBC, 0x0FBD,
    0x0FBE, 0x0FBE, 0x0FBF, 0x0FC0, 0x0FC1, 0x0FC2, 0x0FC3, 0x0FC4, 0x0FC5,
    0x0FC6, 0x0FC7, 0x0FC8, 0x0FC8, 0x0FC9, 0x0FCA, 0x0FCB, 0x0FCC, 0x0FCD,
    0x0FCE, 0x0FCF, 0x0FD0, 0x0FD1, 0x0FD2, 0x0FD3, 0x0FD3, 0x0FD4, 0x0FD5,
    0x0FD6, 0x0FD7, 0x0FD8, 0x0FD9, 0x0FDA, 0x0FDB, 0x0FDC, 0x0FDD, 0x0FDE,
    0x0FDE, 0x0FDF, 0x0FE0, 0x0FE1, 0x0FE2, 0x0FE3, 0x0FE4, 0x0FE5, 0x0FE6,
    0x0FE7, 0x0FE8, 0x0FE9, 0x0FE9, 0x0FEA, 0x0FEB, 0x0FEC, 0x0FED, 0x0FEE,
    0x0FEF, 0x0FF0, 0x0FF1, 0x0FF2, 0x0FF3, 0x0FF4, 0x0FF4, 0x0FF5, 0x0FF6,
    0x0FF7, 0x0FF8, 0x0FF9, 0x0FFA, 0x0FFB, 0x0FFC, 0x0FFD, 0x0FFE, 0x0FFF,
    0x1000, 0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007,
    0x1008, 0x1009, 0x100A, 0x100B, 0x100C, 0x100D, 0x100D, 0x100E, 0x100F,
    0x1010, 0x1011, 0x1012, 0x1013, 0x1014, 0x1015, 0x1016, 0x1017, 0x1018,
    0x1019, 0x101A, 0x101A, 0x101B, 0x101C, 0x101D, 0x101E, 0x101F, 0x1020,
    0x1021, 0x1022, 0x1023, 0x1024, 0x1025, 0x1026, 0x1027, 0x1028, 0x1028,
    0x1029, 0x102A, 0x102B, 0x102C, 0x102D, 0x102E, 0x102F, 0x1030, 0x1031,
    0x1032, 0x1033, 0x1034, 0x1035, 0x1036, 0x1036, 0x1037, 0x1038, 0x1039,
    0x103A, 0x103B, 0x103C, 0x103D, 0x103E, 0x103F, 0x1040, 0x1041, 0x1042,
    0x1043, 0x1044, 0x1045, 0x1045, 0x1046, 0x1047, 0x1048, 0x1049, 0x104A,
    0x104B, 0x104C, 0x104D, 0x104E, 0x104F, 0x1050, 0x1051, 0x1052, 0x1053,
    0x1054, 0x1055, 0x1056, 0x1056, 0x1057, 0x1058, 0x1059, 0x105A, 0x105B,
    0x105C, 0x105D, 0x105E, 0x105F, 0x1060, 0x1061, 0x1062, 0x1063, 0x1064,
    0x1065, 0x1066, 0x1067, 0x1067, 0x1068, 0x1069, 0x106A, 0x106B, 0x106C,
    0x106D, 0x106E, 0x106F, 0x1070, 0x1071, 0x1072, 0x1073, 0x1074, 0x1075,
    0x1076, 0x1077, 0x1078, 0x1079, 0x107A, 0x107A, 0x107B, 0x107C, 0x107D,
    0x107E, 0x107F, 0x1080, 0x1081, 0x1082, 0x1083, 0x1084, 0x1085, 0x1086,
    0x1087, 0x1088, 0x1089, 0x108A, 0x108B, 0x108C, 0x108D, 0x108E, 0x108F,
    0x108F, 0x1090, 0x1091, 0x1092, 0x1093, 0x1094, 0x1095, 0x1096, 0x1097,
    0x1098, 0x1099, 0x109A, 0x109B, 0x109C, 0x109D, 0x109E, 0x109F, 0x10A0,
    0x10A1, 0x10A2, 0x10A3, 0x10A4, 0x10A5, 0x10A6, 0x10A6, 0x10A7, 0x10A8,
    0x10A9, 0x10AA, 0x10AB, 0x10AC, 0x10AD, 0x10AE, 0x10AF, 0x10B0, 0x10B1,
    0x10B2, 0x10B3, 0x10B4, 0x10B5, 0x10B6, 0x10B7, 0x10B8, 0x10B9, 0x10BA,
    0x10BB, 0x10BC, 0x10BD, 0x10BE, 0x10BF, 0x10C0, 0x10C1, 0x10C2, 0x10C2,
    0x10C3, 0x10C4, 0x10C5, 0x10C6, 0x10C7, 0x10C8, 0x10C9, 0x10CA, 0x10CB,
    0x10CC, 0x10CD, 0x10CE, 0x10CF, 0x10D0, 0x10D1, 0x10D2, 0x10D3, 0x10D4,
    0x10D5, 0x10D6, 0x10D7, 0x10D8, 0x10D9, 0x10DA, 0x10DB, 0x10DC, 0x10DD,
    0x10DE, 0x10DF, 0x10E0, 0x10E1, 0x10E2, 0x10E3, 0x10E4, 0x10E5, 0x10E5,
    0x10E6, 0x10E7, 0x10E8, 0x10E9, 0x10EA, 0x10EB, 0x10EC, 0x10ED, 0x10EE,
    0x10EF, 0x10F0, 0x10F1, 0x10F2, 0x10F3, 0x10F4, 0x10F5, 0x10F6, 0x10F7,
    0x10F8, 0x10F9, 0x10FA, 0x10FB, 0x10FC, 0x10FD, 0x10FE, 0x10FF, 0x1100,
    0x1101, 0x1102, 0x1103, 0x1104, 0x1105, 0x1106, 0x1107, 0x1108, 0x1109,
    0x110A, 0x110B, 0x110C, 0x110D, 0x110E, 0x110F, 0x1110, 0x1111, 0x1112,
    0x1113, 0x1114, 0x1115, 0x1116, 0x1117, 0x1118, 0x1119, 0x111A, 0x111B,
    0x111C, 0x111C, 0x111D, 0x111E, 0x111F, 0x1120, 0x1121, 0x1122, 0x1123,
    0x1124, 0x1125, 0x1126, 0x1127, 0x1128, 0x1129, 0x112A, 0x112B, 0x112C,
    0x112D, 0x112E, 0x112F, 0x1130, 0x1131, 0x1132, 0x1133, 0x1134, 0x1135,
    0x1136, 0x1137, 0x1138, 0x1139, 0x113A, 0x113B, 0x113C, 0x113D, 0x113E,
    0x113F, 0x1140, 0x1141, 0x1142, 0x1143, 0x1144, 0x1145, 0x1146, 0x1147,
    0x1148, 0x1149, 0x114A, 0x114B, 0x114C, 0x114D, 0x114E, 0x114F, 0x1150,
    0x1151, 0x1152, 0x1153, 0x1154, 0x1155, 0x1156, 0x1157, 0x1158, 0x1159,
    0x115A, 0x115B, 0x115C, 0x115D, 0x115E, 0x115F, 0x1160, 0x1161, 0x1162,
    0x1163, 0x1164, 0x1165, 0x1166, 0x1167, 0x1168, 0x1169, 0x116A, 0x116B,
    0x116C, 0x116D, 0x116E, 0x116F, 0x1170, 0x1171, 0x1172, 0x1173, 0x1174,
    0x1175, 0x1176, 0x1177, 0x1178, 0x1179, 0x117A, 0x117B, 0x117C, 0x117D,
    0x117E, 0x117F, 0x1180, 0x1181, 0x1183, 0x1184, 0x1185, 0x1186, 0x1187,
    0x1188, 0x1189, 0x118A, 0x118B, 0x118C, 0x118D, 0x118E, 0x118F, 0x1190,
    0x1191, 0x1192, 0x1193, 0x1194, 0x1195, 0x1196, 0x1197, 0x1198, 0x1199,
    0x119A, 0x119B, 0x119C, 0x119D, 0x119E, 0x119F, 0x11A0, 0x11A1, 0x11A2,
    0x11A3, 0x11A4, 0x11A5, 0x11A6, 0x11A7, 0x11A8, 0x11A9, 0x11AA, 0x11AB,
    0x11AC, 0x11AD, 0x11AE, 0x11AF, 0x11B0, 0x11B1, 0x11B2, 0x11B3, 0x11B4,
    0x11B5, 0x11B6, 0x11B7, 0x11B8, 0x11B9, 0x11BA, 0x11BC, 0x11BD, 0x11BE,
    0x11BF, 0x11C0, 0x11C1, 0x11C2, 0x11C3, 0x11C4, 0x11C5, 0x11C6, 0x11C7,
    0x11C8, 0x11C9, 0x11CA, 0x11CB, 0x11CC, 0x11CD, 0x11CE, 0x11CF, 0x11D0,
    0x11D1, 0x11D2, 0x11D3, 0x11D4, 0x11D5, 0x11D6, 0x11D7, 0x11D8, 0x11D9,
    0x11DA, 0x11DB, 0x11DC, 0x11DD, 0x11DE, 0x11E0, 0x11E1, 0x11E2, 0x11E3,
    0x11E4, 0x11E5, 0x11E6, 0x11E7, 0x11E8, 0x11E9, 0x11EA, 0x11EB, 0x11EC,
    0x11ED, 0x11EE, 0x11EF, 0x11F0, 0x11F1, 0x11F2, 0x11F3, 0x11F4, 0x11F5,
    0x11F6, 0x11F7, 0x11F8, 0x11F9, 0x11FA, 0x11FB, 0x11FD, 0x11FE, 0x11FF,
    0x1200, 0x1201, 0x1202, 0x1203, 0x1204, 0x1205, 0x1206, 0x1207, 0x1208,
    0x1209, 0x120A, 0x120B, 0x120C, 0x120D, 0x120E, 0x120F, 0x1210, 0x1211,
    0x1212, 0x1213, 0x1214, 0x1216, 0x1217, 0x1218, 0x1219, 0x121A, 0x121B,
    0x121C, 0x121D, 0x121E, 0x121F, 0x1220, 0x1221, 0x1222, 0x1223, 0x1224,
    0x1225, 0x1226, 0x1227, 0x1228, 0x1229, 0x122A, 0x122C, 0x122D, 0x122E,
    0x122F, 0x1230, 0x1231, 0x1232, 0x1233, 0x1234, 0x1235, 0x1236, 0x1237,
    0x1238, 0x1239, 0x123A, 0x123B, 0x123C, 0x123D, 0x123E, 0x1240, 0x1241,
    0x1242, 0x1243, 0x1244, 0x1245, 0x1246, 0x1247, 0x1248, 0x1249, 0x124A,
    0x124B, 0x124C, 0x124D, 0x124E, 0x124F, 0x1250, 0x1251, 0x1253, 0x1254,
    0x1255, 0x1256, 0x1257, 0x1258, 0x1259, 0x125A, 0x125B, 0x125C, 0x125D,
    0x125E, 0x125F, 0x1260, 0x1261, 0x1262, 0x1263, 0x1265, 0x1266, 0x1267,
    0x1268, 0x1269, 0x126A, 0x126B, 0x126C, 0x126D, 0x126E, 0x126F, 0x1270,
    0x1271, 0x1272, 0x1273, 0x1275, 0x1276, 0x1277, 0x1278, 0x1279, 0x127A,
    0x127B, 0x127C, 0x127D, 0x127E, 0x127F, 0x1280, 0x1281, 0x1282, 0x1283,
    0x1285, 0x1286, 0x1287, 0x1288, 0x1289, 0x128A, 0x128B, 0x128C, 0x128D,
    0x128E, 0x128F, 0x1290, 0x1291, 0x1292, 0x1294, 0x1295, 0x1296, 0x1297,
    0x1298, 0x1299, 0x129A, 0x129B, 0x129C, 0x129D, 0x129E, 0x129F, 0x12A0,
    0x12A2, 0x12A3, 0x12A4, 0x12A5, 0x12A6, 0x12A7, 0x12A8, 0x12A9, 0x12AA,
    0x12AB, 0x12AC, 0x12AD, 0x12AE, 0x12B0, 0x12B1, 0x12B2, 0x12B3, 0x12B4,
    0x12B5, 0x12B6, 0x12B7, 0x12B8, 0x12B9, 0x12BA, 0x12BB, 0x12BC, 0x12BE,
    0x12BF, 0x12C0, 0x12C1, 0x12C2, 0x12C3, 0x12C4, 0x12C5, 0x12C6, 0x12C7,
    0x12C8, 0x12CA, 0x12CB, 0x12CC, 0x12CD, 0x12CE, 0x12CF, 0x12D0, 0x12D1,
    0x12D2, 0x12D3, 0x12D4, 0x12D5, 0x12D7, 0x12D8, 0x12D9, 0x12DA, 0x12DB,
    0x12DC, 0x12DD, 0x12DE, 0x12DF, 0x12E0, 0x12E1, 0x12E3, 0x12E4, 0x12E5,
    0x12E6, 0x12E7, 0x12E8, 0x12E9, 0x12EA, 0x12EB, 0x12EC, 0x12ED, 0x12EF,
    0x12F0, 0x12F1, 0x12F2, 0x12F3, 0x12F4, 0x12F5, 0x12F6, 0x12F7, 0x12F8,
    0x12FA, 0x12FB, 0x12FC, 0x12FD, 0x12FE, 0x12FF, 0x1300, 0x1301, 0x1302,
    0x1303, 0x1304, 0x1306, 0x1307, 0x1308, 0x1309, 0x130A, 0x130B, 0x130C,
    0x130D, 0x130E, 0x130F, 0x1311, 0x1312, 0x1313, 0x1314, 0x1315, 0x1316,
    0x1317, 0x1318, 0x1319, 0x131B, 0x131C, 0x131D, 0x131E, 0x131F, 0x1320,
    0x1321, 0x1322, 0x1323, 0x1324, 0x1326, 0x1327, 0x1328, 0x1329, 0x132A,
    0x132B, 0x132C, 0x132D, 0x132E, 0x1330, 0x1331, 0x1332, 0x1333, 0x1334,
    0x1335, 0x1336, 0x1337, 0x1338, 0x133A, 0x133B, 0x133C, 0x133D, 0x133E,
    0x133F, 0x1340, 0x1341, 0x1342, 0x1344, 0x1345, 0x1346, 0x1347, 0x1348,
    0x1349, 0x134A, 0x134B, 0x134C, 0x134E, 0x134F, 0x1350, 0x1351, 0x1352,
    0x1353, 0x1354, 0x1355, 0x1356, 0x1358, 0x1359, 0x135A, 0x135B, 0x135C,
    0x135D, 0x135E, 0x135F, 0x1361, 0x1362, 0x1363, 0x1364, 0x1365, 0x1366,
    0x1367, 0x1368, 0x1369, 0x136B, 0x136C, 0x136D, 0x136E, 0x136F, 0x1370,
    0x1371, 0x1372, 0x1374, 0x1375, 0x1376, 0x1377, 0x1378, 0x1379, 0x137A,
    0x137B, 0x137D, 0x137E, 0x137F, 0x1380, 0x1381, 0x1382, 0x1383, 0x1384,
    0x1386, 0x1387, 0x1388, 0x1389, 0x138A, 0x138B, 0x138C, 0x138E, 0x138F,
    0x1390, 0x1391, 0x1392, 0x1393, 0x1394, 0x1395, 0x1397, 0x1398, 0x1399,
    0x139A, 0x139B, 0x139C, 0x139D, 0x139E, 0x13A0, 0x13A1, 0x13A2, 0x13A3,
    0x13A4, 0x13A5, 0x13A6, 0x13A8, 0x13A9, 0x13AA, 0x13AB, 0x13AC, 0x13AD,
    0x13AE, 0x13B0, 0x13B1, 0x13B2, 0x13B3, 0x13B4, 0x13B5, 0x13B6, 0x13B7,
    0x13B9, 0x13BA, 0x13BB, 0x13BC, 0x13BD, 0x13BE, 0x13BF, 0x13C1, 0x13C2,
    0x13C3, 0x13C4, 0x13C5, 0x13C6, 0x13C7, 0x13C9, 0x13CA, 0x13CB, 0x13CC,
    0x13CD, 0x13CE, 0x13CF, 0x13D1, 0x13D2, 0x13D3, 0x13D4, 0x13D5, 0x13D6,
    0x13D7, 0x13D9, 0x13DA, 0x13DB, 0x13DC, 0x13DD, 0x13DE, 0x13E0, 0x13E1,
    0x13E2, 0x13E3, 0x13E4, 0x13E5, 0x13E6, 0x13E8, 0x13E9, 0x13EA, 0x13EB,
    0x13EC, 0x13ED, 0x13EE, 0x13F0, 0x13F1, 0x13F2, 0x13F3, 0x13F4, 0x13F5,
    0x13F7, 0x13F8, 0x13F9, 0x13FA, 0x13FB, 0x13FC, 0x13FD, 0x13FF, 0x1400,
    0x1401, 0x1402, 0x1403, 0x1404, 0x1406, 0x1407, 0x1408, 0x1409, 0x140A,
    0x140B, 0x140C, 0x140E, 0x140F, 0x1410, 0x1411, 0x1412, 0x1413, 0x1415,
    0x1416, 0x1417, 0x1418, 0x1419, 0x141A, 0x141C, 0x141D, 0x141E, 0x141F,
    0x1420, 0x1421, 0x1423, 0x1424, 0x1425, 0x1426, 0x1427, 0x1428, 0x142A,
    0x142B, 0x142C, 0x142D, 0x142E, 0x142F, 0x1431, 0x1432, 0x1433, 0x1434,
    0x1435, 0x1436, 0x1438, 0x1439, 0x143A, 0x143B, 0x143C, 0x143D, 0x143F,
    0x1440, 0x1441, 0x1442, 0x1443, 0x1444, 0x1446, 0x1447, 0x1448, 0x1449,
    0x144A, 0x144B, 0x144D, 0x144E, 0x144F, 0x1450, 0x1451, 0x1452, 0x1454,
    0x1455, 0x1456, 0x1457, 0x1458, 0x145A, 0x145B, 0x145C, 0x145D, 0x145E,
    0x145F, 0x1461, 0x1462, 0x1463, 0x1464, 0x1465, 0x1466, 0x1468, 0x1469,
    0x146A, 0x146B, 0x146C, 0x146E, 0x146F, 0x1470, 0x1471, 0x1472, 0x1473,
    0x1475, 0x1476, 0x1477, 0x1478, 0x1479, 0x147B, 0x147C, 0x147D, 0x147E,
    0x147F, 0x1480, 0x1482, 0x1483, 0x1484, 0x1485, 0x1486, 0x1488, 0x1489,
    0x148A, 0x148B, 0x148C, 0x148D, 0x148F, 0x1490, 0x1491, 0x1492, 0x1493,
    0x1495, 0x1496, 0x1497
};
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
double ConvertPercentAmplitudeToAttenDB_SF2(double percent) {
  if (percent == 0)
    return 100.0;        // assume 0 is -100.0db attenuation
  double atten = 20 * log10(percent);
  return !(-atten<100.0)?-atten:100.0;
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
void merge_sample_roms(UINT8 ** buffer_ptr){

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
    return;
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
    UINT32 num_samples = (size / 16) + 1;
    struct sample cps3_samples[num_samples];
    for (pos = 0; pos < size; pos += 16){
        num_samples = pos / 16;
        cps3_samples[num_samples].start = ((data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3]);
        cps3_samples[num_samples].loop = ((data[pos + 4] << 24) | (data[pos + 5] << 16) | (data[pos + 6] << 8) | data[pos + 7]);
        cps3_samples[num_samples].end = ((data[pos + 8] << 24) | (data[pos + 9] << 16) | (data[pos + 10] << 8) | data[pos + 11]);
        cps3_samples[num_samples].size = cps3_samples[num_samples].end - cps3_samples[num_samples].start;
        //if (cps3_samples[num_samples].loop >= cps3_samples[num_samples].end) cps3_samples[num_samples].size -= 46;
        cps3_samples[num_samples].loop = cps3_samples[num_samples].loop - cps3_samples[num_samples].start;
        cps3_samples[num_samples].key = data[pos + 15];

    }
    printf("total samples: %x", num_samples);
    int tot_samples = num_samples + 2;
    sfSample sf2_sample[tot_samples + 1];
    memset(sf2_sample, 0, tot_samples + 1);
    *root_key_array = malloc(tot_samples);
    memset(*root_key_array, 0, tot_samples);
    sfSample * tmp_sample;
    UINT8 * buffer;
    merge_sample_roms(&buffer);
    UINT32 sample_rom_pos = 0;
    UINT32 tot_size = cps3_samples[num_samples].end ;
    UINT8 * wav_data = malloc(sizeof(char) * tot_size);
    UINT16 * smpdata = malloc(sizeof(INT16) * (tot_size + (tot_samples * 46)));
    INT64 check = sizeof(INT16) * (tot_size + (tot_samples * 46));
    UINT32 SmplHdrSize = sizeof(sfSample) * (tot_samples + 2);
    UINT64 sample_size;
    UINT64 sample_start;
    UINT64 sample_end;
    UINT64 sample_loop;
    UINT8 sample_key;
    UINT32 sf2_smp_pos = 0;
    *RetLoopMsk = (UINT8*)malloc(pos);
	memset(*RetLoopMsk, 0x00, pos);
    for (num_samples = 0; num_samples < tot_samples - 1; num_samples ++){
        sample_size = cps3_samples[num_samples].size;
        sample_start = cps3_samples[num_samples].start;
        sample_end = cps3_samples[num_samples].end;
        sample_key = cps3_samples[num_samples].key;
        sample_loop = cps3_samples[num_samples].loop;
        (*root_key_array)[num_samples] = sample_key - 26;
        tmp_sample = &sf2_sample[num_samples];
        memset(tmp_sample, 0x00, sizeof(sfSample));
        sprintf(tmp_sample->achSampleName, "Sample %d", num_samples);
        sf2_sample[num_samples].dwSampleRate = 8200;
        sf2_sample[num_samples].byOriginalKey = sample_key;
        sf2_sample[num_samples].chCorrection = 0;
		sf2_sample[num_samples].wSampleLink = 0;
		sf2_sample[num_samples].sfSampleType = monoSample;
		sf2_sample[num_samples].dwStart = cps3_samples[num_samples].start + (num_samples * 46);
		sf2_sample[num_samples].dwEnd = cps3_samples[num_samples].end + (num_samples * 46);
		sf2_sample[num_samples].dwStartloop = cps3_samples[num_samples].start + cps3_samples[num_samples].loop + (num_samples * 46);
        sf2_sample[num_samples].dwEndloop = cps3_samples[num_samples].end + (num_samples * 46);
        if  (sample_loop < sample_end)
            (*RetLoopMsk)[num_samples >> 3] |= 1 << (num_samples & 0x07);
        for (; sf2_smp_pos < sample_end + (num_samples * 46); sample_rom_pos ++, sf2_smp_pos++){
            smpdata[sf2_smp_pos] = (INT8)buffer[sample_rom_pos] * 0x100;
            if (sf2_smp_pos == 4648){
                UINT8 check = 1;
            }
        }
        for (int extra_samples_pos = 0; extra_samples_pos < 46; extra_samples_pos ++, sf2_smp_pos++){
            smpdata[sf2_smp_pos] = 0x00;
        }
    }
    tmp_sample = &sf2_sample[num_samples];
	memset(tmp_sample, 0x00, sizeof(sfSample));
	sprintf(tmp_sample->achSampleName, "EOS");
    UINT32 SmplDBSize = sizeof(INT16) * (tot_size + (num_samples * 46));
	SmplHdrSize = sizeof(sfSample) * (tot_samples);
	LIST_CHUNK * LstChk = List_GetChunk(SF2Data->Lists, FCC_sdta);
	ITEM_CHUNK * ItmChk = Item_MakeChunk(FCC_smpl, SmplDBSize, smpdata, 0x00);
	List_AddItem(LstChk, ItmChk);
	LstChk = List_GetChunk(SF2Data->Lists, FCC_pdta);
	ItmChk = Item_MakeChunk(FCC_shdr, SmplHdrSize, sf2_sample, 0x00);	// no free() needed either
	List_AddItem(LstChk, ItmChk);
    return tot_samples;
}
const void env_fix(INT16 *atk, INT16 *dec, INT16 *sus, INT16 *srt, INT16 *rel){
    //from vgmtrans source code (QsoundInstr.h)
    //i slightly modified it, but i still copied from it
    //because i have been informed that the envelope table found in the cps3 was similar to the one in the cps2

    //actually, im going to "disassemble" the code for the envelopes, and sort-of compare them...
    //not happy with the result i have now, also the mixing is bad...
    INT16 ticks = 0;
    INT16 atk_rate = 0;
    INT16 dec_rate = 0;
    INT16 sus_levl = 0;
    INT16 srt_rate = 0;
    INT16 rel_rate = 0;
    rate_convert(*atk, 2, &atk_rate);
    rate_convert(*dec, 1, &dec_rate);
    rate_convert(*sus, 0, &sus_levl);
    rate_convert(*srt, 1, &srt_rate);
    rate_convert(*rel, 1, &rel_rate);
    //ATTACK ENVELOPE
    if (atk_rate == 0xffff) atk_rate = -32768;
    else {
        ticks = (0xffff / atk_rate) * cps3_tick;
        atk_rate = 1200 * Log2(ticks);
    }
    //DECAY ENVELOPE
     if (sus_levl >= 0x7E && srt_rate > 0 && dec_rate > 1) {
        ticks = (long) ceil((0xFFFF - sus_levl) / (double) dec_rate);
        ticks += (long) ceil(sus_levl / (double) srt_rate);
        dec_rate = ticks * cps3_tick;
        sus_levl = 0;
    }
    else {
        ticks = dec_rate ? (0xFFFF / dec_rate) : 0;
        dec_rate = (dec_rate == 0xFFFF) ? 0 : ticks * cps3_tick;
    }
    dec_rate = LinAmpDecayTimeToLinDBDecayTime(dec_rate, 0x800);
    dec_rate = 1200 * Log2(dec_rate);
    //SUSTAIN VOLUME
    if (dec_rate <= 1) sus_levl = 1.0;
    else sus_levl = sus_levl / (double) 0xFFFF;
    if (sus_levl == -1)
        sus_levl = 0.0;
    else
        sus_levl = ConvertPercentAmplitudeToAttenDB_SF2(sus_levl);
    if (sus_levl > 100.0)
        sus_levl = 100.0;
    //SUSTAIN ENVELOPE
    //(unused?)
    ticks = srt_rate ? 0xFFFF / srt_rate : 0;
    srt_rate = (srt_rate == 0xFFFF) ? 0 : ticks * cps3_tick;
    srt_rate = LinAmpDecayTimeToLinDBDecayTime(srt_rate, 0x800);
    srt_rate = 1200 * Log2(srt_rate);
    //RELEASE ENVELOPE
    ticks = rel_rate ? 0xFFFF / rel_rate : 0;
    rel_rate = (rel_rate == 0xFFFF) ? 0 : ticks * cps3_tick;
    rel_rate = LinAmpDecayTimeToLinDBDecayTime(rel_rate, 0x800);
    rel_rate = 1200 * Log2(rel_rate);
    *atk = RoundTo16(atk_rate);
    *dec = RoundTo16(dec_rate);
    *sus = RoundTo16(sus_levl);
    *srt = RoundTo16(srt_rate);
    *rel = RoundTo16(rel_rate);
    //
    return;
}
UINT16 GenerateInstruments(SF2_DATA* SF2Data, UINT16 SmplCnt, const UINT8* LoopMsk, UINT8* root_key_array, UINT8** is_instr_null){
    FILE * instrumentfile = fopen("instrument.bin", "rb");
    if (instrumentfile == NULL){
        printf("error opening instrument file");
        exit(EXIT_FAILURE);
    }
    fseek(instrumentfile, 0, SEEK_END);
    int size = ftell(instrumentfile);
    printf("the size of instrument rom is %x\n", size);
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
    for (int i = 0; i < instr_count; i ++){
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
                if (CurIns == 42){
                    CHAR check = 1;
                }
            memset(&InsData[CurIns], 0x00, sizeof(sfInst));
            sprintf(InsData[CurIns].achInstName, "Instrument %02hX", CurIns);
            InsData[CurIns].wInstBagNdx = InsBagCnt;
                for(pos = bank_offset + inst_offset; end_instr < 0xffff; pos += 12){
                    CurNote = data[pos];
                    if (CurNote < LastNote) break;
                    UINT16 sample_id = (data[pos + 4] << 8) | data[pos + 5];
                    UINT8 key = root_key_array[sample_id];
                    UINT8 loop_mode; //loop_mode = (LoopMsk[sample_id >> 3] & (1 << (sample_id & 0x07))) ? 1 : 0;
                    if (LoopMsk[sample_id >> 3] & (1 << (sample_id & 0x07)))
                        loop_mode = 0x01;	// Loop on
                    else
                        loop_mode = 0x00; // Loop off
                    INT8 VOL = data[pos + 1];
                    INT16 atk = data[pos + 7];//attack
                    INT16 dec = data[pos + 8];//decay
                    INT16 sus = data[pos + 9];//sustain level
                    INT16 srt = data[pos + 10];//sustain rate
                    INT16 rel = data[pos + 11];//release
                    env_fix(&atk, &dec, &sus, &srt, &rel);
                    AddInsBag(&InsBagAlloc, &InsBagCnt, &InsBags, InsGenCnt, 0);
                    AddInsGen_8(&InsGenAlloc, &InsGenCnt, &InsGen, keyRange, LastNote + 1, CurNote);
                    AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, overridingRootKey, key);
                    AddInsGen_U16(&InsGenAlloc, &InsGenCnt, &InsGen, sampleModes, loop_mode);
                    AddInsGen_U16(&InsGenAlloc, &InsGenCnt, &InsGen, sampleID, sample_id);
                    AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, attackVolEnv, atk); //evndata & 0x7fff
                    AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, decayVolEnv, dec);
                    AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, sustainVolEnv, sus);
                    AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, releaseVolEnv, rel);
                    //AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, fineTune, -32);
                    //AddInsGen_S16(&InsGenAlloc, &InsGenCnt, &InsGen, velocity, srt);
                    end_instr = (data[pos + 12] << 8) | data[pos + 13];
                    for (int note = LastNote; note <= CurNote; note ++){
                        INT8 vib_sensitivity = data[pos + 6];
                        key_split[CurIns][note].sample_key = key;
                        key_split[CurIns][note].vib_sensitivity = vib_sensitivity;
                        //printf("\n instrument: %x , note: %x , sample key: %x , vibrato_sensitivity: %x", CurIns, note, key, data[pos + 6]);
                    }
                    LastNote = CurNote;
                }
            } else{
                memset(&InsData[CurIns], 0x00, sizeof(sfInst));
                sprintf(InsData[CurIns].achInstName, "empty instrument", CurIns);
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
		sprintf(TempPHdr->achPresetName, "preset %02hd", CurIns);
		if (is_instr_null[CurIns] == 1) sprintf(TempPHdr->achPresetName, "empty preset", CurIns);
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
		sprintf(TempPHdr->achPresetName, "channel 10 preset %02hd", CurIns);
		if (is_instr_null[CurIns] == 1) sprintf(TempPHdr->achPresetName, "empty preset", CurIns);
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
const void make_soundfont(const char* FileName, UINT8 bank_to_copy){
    SF2_DATA* SF2Data;
    UINT16 SmplCnt;
	UINT16 InsCnt;
	UINT8* is_instr_null;
    UINT8* SmplLoopMask;
	UINT8 RetVal;
    UINT8* root_key_array;
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
const void analyze_song(UINT8* data, UINT32 pos, UINT8 master_channel, UINT8 sequence){
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
    UINT32 song_lenght = 0;
    UINT32 chn_size = 0;
    UINT8 start_loop_times = 0;
    UINT16 max_chn;
    UINT8 vib_cnt = 0;
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
                        case 0xc2:case 0xc3:case 0xc4:case 0xc5:case 0xc6:case 0xc7:case 0xc8:case 0xc9:case 0xe0:case 0xe1:case 0xe2:case 0xe6:case 0xe7:{
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
                                loop_start_type = type;
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
                            if(cps3_repeat[type].times == 0){
                                cps3_repeat[type].times = data[pos + 1];
                                if (cps3_repeat[type].times == 0 || cps3_repeat[type].times >= 0x7e){
                                        //forgot to put the skip, so here it is.
                                        pos += 2;
                                        if (type != loop_start_type){
                                            chn_loop_start[temp_chn_count] = temp_loop_start[loop_start_type];
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
        }
        else temp_chn_count ++;
    }
    for (temp_chn_count = 0; temp_chn_count < max_chn; temp_chn_count ++){
        if (song_lenght < chn_loop_size[temp_chn_count]){
            song_lenght = chn_loop_size[temp_chn_count];
            master_channel = temp_chn_count;
        }
    }
    UINT16 min_start = 0xffff;
    for (temp_chn_count = 0; temp_chn_count < max_chn; temp_chn_count ++){
        if (chn_loop_size[temp_chn_count] == 0) chn_loop_size[temp_chn_count] = song_lenght;
        if (chn_loop_size[temp_chn_count] == song_lenght && min_start > chn_loop_start[temp_chn_count]){
            min_start = chn_loop_start[temp_chn_count];
            master_channel = temp_chn_count;
        }
    }
    float loop_times;
    //printf("\nsong lenght is %x\n", song_lenght);
    for (temp_chn_count = 0; temp_chn_count < max_chn; temp_chn_count ++){
        if (temp_chn_count == 11 && sequence == 15){
            UINT8 check1 = 1;
        }
        if (chn_loop_size[temp_chn_count] < song_lenght){
            result_loop_times[temp_chn_count] = (song_lenght / chn_loop_size[temp_chn_count]);
            loop_times = result_loop_times[temp_chn_count];
            result_loop_times[temp_chn_count] *= 3;
            loop_times = result_loop_times[temp_chn_count];
            result_loop_times[temp_chn_count] -= 1;
            loop_times = result_loop_times[temp_chn_count];
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
    return;
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
    vib_depth = 0;
    lfo_state = 1;
    cur_lfo_val = 0;
    lfo_increment = 0;
}
const void portamento_fix(float semitone, UINT8 new_rpn){
    INT16 portamento = semitone * 8192 / new_rpn;
    return;
}

const void make_song(UINT8* data, UINT32 pos, UINT8 master_channel, UINT8 sequence){
    float msec_tick = 0;
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
    UINT8 note;
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
    float lfo_tick = 0;
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
    UINT8 bank_number = 0xff;
    INT32 vibrato_value;
    UINT8 check = 0;
    INT8 pitch = 0;
    UINT32 bend_tick = 0;
    UINT8 bend_on = 0;
    UINT32 seq_offset = ((data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3]);
    for (pos = seq_offset + 1; pos < seq_offset + 33 ; pos += 2){
        UINT16 chn_offset = ((data[pos] << 8) | data[pos + 1]);
        if (chn_offset != 0x0000){
            UINT32 tot_tick = 0;
            ////printf("\n in channel %d(offset %x) \n\t", channel, chn_offset + seq_offset);
            mid_state.midChn = mid_state.midChn & 0x0f;
            float cur_lfo_tick = 0;
            WriteMidiTrackStart(&midi_inf, &mid_state);
            WriteEvent(&midi_inf, &mid_state, 0xB0| mid_state.midChn, 0x7E, 00);
            WriteEvent(&midi_inf, &mid_state, 0xB0| mid_state.midChn, 0x7D, 00);
            if (sequence == 9){
                check = 1;
            }
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
            for (pos = seq_offset + chn_offset; data[pos] != 0xff; ){
                if (mid_state.midChn == 0x5){
                    UINT8 check2 = 1;
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
                        note = data[pos + 1];
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
                                if (vib_on == 1){
                                    lfo_increment = vib_depth * lfo_rate;
                                    vib_depth <<= 16;
                                }
                                else if(play_mode == 1){
                                    reset_lfo(&vib_depth, &lfo_rate, &lfo_state, &lfo_limit, &cur_lfo_val, &lfo_increment);
                                    WriteEvent(&midi_inf, &mid_state, 0xe0, 0x00, 0x40);
                                }
                                pos += 2;
                                break;
                            }
                            case 0xc6:{ //CHANNEL VOLUME
                                //printf("0xc6 in pos %x", pos);
                                WriteEvent(&midi_inf, &mid_state, 0xb0, 7, data[pos + 1] & 0x7f);
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
                                WriteEvent(&midi_inf, &mid_state, 0xb0, 11, data[pos + 1] & 0x7f);
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
                                if(cps3_repeat[type].times == 0){
                                    cps3_repeat[type].times = data[pos + 1]; //reads how many times this repeat command should be processed
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
                            INT8 vibrato_sensivity = key_split[instr_number][note].vib_sensitivity;
                            key_fraction = (note - sample_key) + 7;
                            key_fraction = (key_fraction << 8) + 0x80 + vibrato_sensivity;
                            INT32 target_bend = key_fraction; */
                            INT32 bend_final;
                            bend_final = ((note_bend_final) << 6);
                            if (vib_on){
                                vibrato_final = (cur_lfo_val >> 16) << 1;
                                bend_final += (vibrato_final);
                            }
                            if (bend_final != prev_bend){
                                prev_bend = bend_final;
                                bend_final += 0x2000;
                                WriteEvent(&midi_inf, &mid_state, 0xe0 | mid_state.midChn, bend_final & 0x7f, (bend_final >> 7) & 0x7f);
                            }
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
    return;
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
        analyze_song(data, pos, master_channel, song_id);
        //then, make the midi
        make_song(data, pos, master_channel, song_id);
        //and, finally, write the song to a new midi file
        printf("done\n");
        pos = seq_table_pos;
        song_id ++; //go to the next song
    }
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

