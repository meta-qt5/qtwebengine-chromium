//===-- Processors.td - TODO: Add brief description -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// AMDIL processors supported.
//
//===----------------------------------------------------------------------===//

class Proc<string Name, ProcessorItineraries itin, list<SubtargetFeature> Features> 
: Processor<Name, itin, Features>;
def : Proc<"rv710",      R600_EG_Itin, []>;
def : Proc<"rv730",      R600_EG_Itin, []>;
def : Proc<"rv770",      R600_EG_Itin, [FeatureFP64]>;
def : Proc<"cedar",      R600_EG_Itin, [FeatureByteAddress, FeatureImages]>;
def : Proc<"redwood",    R600_EG_Itin, [FeatureByteAddress, FeatureImages]>;
def : Proc<"juniper",    R600_EG_Itin, [FeatureByteAddress, FeatureImages]>;
def : Proc<"cypress",    R600_EG_Itin, [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"barts",      R600_EG_Itin, [FeatureByteAddress, FeatureImages]>;
def : Proc<"turks",      R600_EG_Itin, [FeatureByteAddress, FeatureImages]>;
def : Proc<"caicos",     R600_EG_Itin, [FeatureByteAddress, FeatureImages]>;
def : Proc<"cayman",     R600_EG_Itin, [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"SI", SI_Itin, [Feature64BitPtr]>;

