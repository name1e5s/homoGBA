/*
 * This file is part of homoGBA.
 * Copyright (C) 2019  name1e5s<name1e5s@qq.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ARM_TABLE_GENERATOR_H
#define ARM_TABLE_GENERATOR_H
#include <macros.h>
// It's difficult to find whether the final table
// is Okay. So we use magic to generate it
// automatically.
#define DECL_INSN_ARM(PREFIX, INSN) PREFIX##_##INSN

#define DECL_ALU_INSN_ARM(PREFIX, ALU, OTHER1, OTHER2, OTHER3, OTHER4) \
  REP_2(DECL_INSN_ARM(PREFIX, ALU##_LSL)),                             \
      REP_2(DECL_INSN_ARM(PREFIX, ALU##_LSR)),                         \
      REP_2(DECL_INSN_ARM(PREFIX, ALU##_ASR)),                         \
      REP_2(DECL_INSN_ARM(PREFIX, ALU##_ROR)),                         \
      DECL_INSN_ARM(PREFIX, ALU##_LSL), DECL_INSN_ARM(PREFIX, OTHER1), \
      DECL_INSN_ARM(PREFIX, ALU##_LSR), DECL_INSN_ARM(PREFIX, OTHER2), \
      DECL_INSN_ARM(PREFIX, ALU##_ASR), DECL_INSN_ARM(PREFIX, OTHER3), \
      DECL_INSN_ARM(PREFIX, ALU##_ROR), DECL_INSN_ARM(PREFIX, OTHER4)

#define DECL_ALU_INSN_IMMI(PREFIX, ALU) REP_16(DECL_INSN_ARM(PREFIX, ALU##_I))

#define GENERATE_ARM_INSN_TABLE(PREFIX)                                       \
  DECL_ALU_INSN_ARM(PREFIX, AND, MUL, STRH, ILL, ILL),                        \
      DECL_ALU_INSN_ARM(PREFIX, AND_S, MUL_S, LDRH, LDRSB, LDRSH),            \
      DECL_ALU_INSN_ARM(PREFIX, EOR, MLA, STRH, ILL, ILL),                    \
      DECL_ALU_INSN_ARM(PREFIX, EOR_S, MLA_S, LDRH, LDRSB, LDRSH),            \
      DECL_ALU_INSN_ARM(PREFIX, SUB, ILL, STRH_I, ILL, ILL),                  \
      DECL_ALU_INSN_ARM(PREFIX, SUB_S, ILL, LDRH_I, LDRSB_I, LDRSH_I),        \
      DECL_ALU_INSN_ARM(PREFIX, RSB, ILL, STRH_I, ILL, ILL),                  \
      DECL_ALU_INSN_ARM(PREFIX, RSB_S, ILL, LDRH_I, LDRSB_I, LDRSH_I),        \
      DECL_ALU_INSN_ARM(PREFIX, ADD, UMULL, STRHU, ILL, ILL),                 \
      DECL_ALU_INSN_ARM(PREFIX, ADD_S, UMULL_S, LDRH_U, LDRSB_U, LDRSH_U),    \
      DECL_ALU_INSN_ARM(PREFIX, ADC, UMLAL, STRH_U, ILL, ILL),                \
      DECL_ALU_INSN_ARM(PREFIX, ADC_S, UMLAL_S, LDRH_U, LDRSB_U, LDRSH_U),    \
      DECL_ALU_INSN_ARM(PREFIX, SBC, SMULL, STRHIU, ILL, ILL),                \
      DECL_ALU_INSN_ARM(PREFIX, SBC_S, SMULL_S, LDRH_IU, LDRSB_IU, LDRSH_IU), \
      DECL_ALU_INSN_ARM(PREFIX, RSC, SMLAL, STRH_IU, ILL, ILL),               \
      DECL_ALU_INSN_ARM(PREFIX, RSC_S, SMLAL_S, LDRH_IU, LDRSB_IU, LDRSH_IU), \
      DECL_INSN_ARM(PREFIX, MRS), REP_8(DECL_INSN_ARM(PREFIX, ILL)),          \
      DECL_INSN_ARM(PREFIX, SWP), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, STRH_P), REP_4(DECL_INSN_ARM(PREFIX, ILL)),       \
      DECL_ALU_INSN_ARM(PREFIX, TST, ILL, LDRH_P, LDRSB_P, LDRSH_P),          \
      DECL_INSN_ARM(PREFIX, MSR), DECL_INSN_ARM(PREFIX, BX),                  \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, BKPT),                \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, STRH_PW),             \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_ALU_INSN_ARM(PREFIX, TEQ, ILL, LDRH_PW, LDRSB_PW, LDRSH_PW),       \
      DECL_INSN_ARM(PREFIX, MRSR), DECL_INSN_ARM(PREFIX, ILL),                \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, SWPB),                \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, STRH_IP),             \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_ALU_INSN_ARM(PREFIX, CMP, ILL, LDRH_IP, LDRSB_IP, LDRSH_IP),       \
      DECL_INSN_ARM(PREFIX, MSRR), DECL_INSN_ARM(PREFIX, ILL),                \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, STRH_IPW),            \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_INSN_ARM(PREFIX, ILL), DECL_INSN_ARM(PREFIX, ILL),                 \
      DECL_ALU_INSN_ARM(PREFIX, CMN, ILL, LDRH_IPW, LDRSB_IPW, LDRSH_IPW),    \
      DECL_ALU_INSN_ARM(PREFIX, ORR, SMLAL, STRH_PU, ILL, ILL),               \
      DECL_ALU_INSN_ARM(PREFIX, ORR_S, SMLAL_S, LDRH_PU, LDRSB_PU, LDRSH_PU), \
      DECL_ALU_INSN_ARM(PREFIX, MOV, SMLAL, STRH_PUW, ILL, ILL),              \
      DECL_ALU_INSN_ARM(PREFIX, MOV_S, SMLAL_S, LDRH_PUW, LDRSB_PUW,          \
                        LDRSH_PUW),                                           \
      DECL_ALU_INSN_ARM(PREFIX, BIC, SMLAL, STRH_IPU, ILL, ILL),              \
      DECL_ALU_INSN_ARM(PREFIX, BIC_S, SMLAL_S, LDRH_IPU, LDRSB_IPU,          \
                        LDRSH_IPU),                                           \
      DECL_ALU_INSN_ARM(PREFIX, MVN, SMLAL, STRH_IPUW, ILL, ILL),             \
      DECL_ALU_INSN_ARM(PREFIX, MVN_S, SMLAL_S, LDRH_IPUW, LDRSB_IPUW,        \
                        LDRSH_IPUW),                                          \
      DECL_ALU_INSN_IMMI(PREFIX, AND), DECL_ALU_INSN_IMMI(PREFIX, AND_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, EOR), DECL_ALU_INSN_IMMI(PREFIX, EOR_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, SUB), DECL_ALU_INSN_IMMI(PREFIX, SUB_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, RSB), DECL_ALU_INSN_IMMI(PREFIX, RSB_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, ADD), DECL_ALU_INSN_IMMI(PREFIX, ADD_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, ADC), DECL_ALU_INSN_IMMI(PREFIX, ADC_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, SBC), DECL_ALU_INSN_IMMI(PREFIX, SBC_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, RSC), DECL_ALU_INSN_IMMI(PREFIX, RSC_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, TST), DECL_ALU_INSN_IMMI(PREFIX, TST),       \
      DECL_ALU_INSN_IMMI(PREFIX, MSR), DECL_ALU_INSN_IMMI(PREFIX, TEQ),       \
      DECL_ALU_INSN_IMMI(PREFIX, CMP), DECL_ALU_INSN_IMMI(PREFIX, CMP),       \
      DECL_ALU_INSN_IMMI(PREFIX, MSRR), DECL_ALU_INSN_IMMI(PREFIX, CMN),      \
      DECL_ALU_INSN_IMMI(PREFIX, ORR), DECL_ALU_INSN_IMMI(PREFIX, ORR_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, MOV), DECL_ALU_INSN_IMMI(PREFIX, MOV_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, BIC), DECL_ALU_INSN_IMMI(PREFIX, BIC_S),     \
      DECL_ALU_INSN_IMMI(PREFIX, MVN), DECL_ALU_INSN_IMMI(PREFIX, MVN_S)
// TODO:::

#endif
