/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with 
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 */

#include "MatrixCHOP.h"

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillCHOPPluginInfo(CHOP_PluginInfo *info)
{
	// Always set this to CHOPCPlusPlusAPIVersion.
	info->apiVersion = CHOPCPlusPlusAPIVersion;

	// The opType is the unique name for this CHOP. It must start with a 
	// capital A-Z character, and all the following characters must lower case
	// or numbers (a-z, 0-9)
	info->customOPInfo.opType->setString("Matrixchop");

	// The opLabel is the text that will show up in the OP Create Dialog
	info->customOPInfo.opLabel->setString("Matrix CHOP");

	// Information about the author of this OP
	info->customOPInfo.authorName->setString("David Braun");
	info->customOPInfo.authorEmail->setString("github.com/dbraun");

	// This CHOP can work with 0 inputs
	info->customOPInfo.minInputs = 2;

	// It can accept up to 1 input though, which changes it's behavior
	info->customOPInfo.maxInputs = 2;
}

DLLEXPORT
CHOP_CPlusPlusBase*
CreateCHOPInstance(const OP_NodeInfo* info)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per CHOP that is using the .dll
	return new MatrixCHOP(info);
}

DLLEXPORT
void
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the CHOP using that instance is deleted, or
	// if the CHOP loads a different DLL
	delete (MatrixCHOP*)instance;
}

};


MatrixCHOP::MatrixCHOP(const OP_NodeInfo* info) : myNodeInfo(info)
{
	myExecuteCount = 0;
	myOffset = 0.0;
}

MatrixCHOP::~MatrixCHOP()
{

}

void
MatrixCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = false;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	ginfo->timeslice = false;

	ginfo->inputMatchIndex = 0;
}

bool
MatrixCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.

	return false;

	if (inputs->getNumInputs() > 0)
	{
		return false;
	}
	else
	{
		info->numChannels = 1;

		// Since we are outputting a timeslice, the system will dictate
		// the numSamples and startIndex of the CHOP data
		//info->numSamples = 1;
		//info->startIndex = 0

		// For illustration we are going to output 120hz data
		info->sampleRate = 60;
		return true;
	}
}

void
MatrixCHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
	name->setString("chan1");
}

void
MatrixCHOP::execute(CHOP_Output* output,
							  const OP_Inputs* inputs,
							  void* reserved)
{
	myExecuteCount++;
	
	double	 scale = inputs->getParDouble("Scale");

	// In this case we'll just take the first input and re-output it scaled.

	if (inputs->getNumInputs() > 1)
	{
		// We know the first CHOP has the same number of channels
		// because we returned false from getOutputInfo.

		const OP_CHOPInput* valuesInput = inputs->getInputCHOP(0);
		const OP_CHOPInput* matrixInput = inputs->getInputCHOP(1);


		if (matrixInput->numChannels < 15) {
			return;
		}

		for (int i = 0; i < 16; i++) {
			theMat.vals[i/4][i%4] = matrixInput->getChannelData(i)[0];
		}

		int ind = 0;

		float tx = 0.f;
		float ty = 0.f;
		float tz = 0.f;

		if (valuesInput->numChannels < 3) {
			return;
		}

		for (int i = 0; i < valuesInput->numSamples; i++)
		{

			float tx = float(valuesInput->getChannelData(0)[i]);
			float ty = float(valuesInput->getChannelData(1)[i]);
			float tz = float(valuesInput->getChannelData(2)[i]);

			float theVec[4] = {
				tx,
				ty,
				tz,
				1.
			};

			float* newJoint = theMat.multByVector(theVec);

			newJoint[0] /= newJoint[3];
			newJoint[1] /= newJoint[3];
			//newJoint[2] /= newJoint[3];

			newJoint[0] = .5 + newJoint[0] * .5;
			newJoint[1] = .5 + newJoint[1] * .5;
			newJoint[2] = .5 + newJoint[2] * .5;

			output->channels[0][i] = newJoint[0];
			output->channels[1][i] = newJoint[1];
			output->channels[2][i] = newJoint[2];
		}
		
	}
}

int32_t
MatrixCHOP::getNumInfoCHOPChans(void * reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 2;
}

void
MatrixCHOP::getInfoCHOPChan(int32_t index,
										OP_InfoCHOPChan* chan,
										void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)myOffset;
	}
}

bool		
MatrixCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 2;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
MatrixCHOP::getInfoDATEntries(int32_t index,
										int32_t nEntries,
										OP_InfoDATEntries* entries, 
										void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0)
	{
		// Set the value for the first column
		entries->values[0]->setString("executeCount");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#else // macOS
        snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		// Set the value for the first column
		entries->values[0]->setString("offset");

		// Set the value for the second column
#ifdef _WIN32
        sprintf_s(tempBuffer, "%g", myOffset);
#else // macOS
        snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString( tempBuffer);
	}
}

void
MatrixCHOP::setupParameters(OP_ParameterManager* manager, void *reserved1)
{
	// speed
	{
		OP_NumericParameter	np;

		np.name = "Speed";
		np.label = "Speed";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = -10.0;
		np.maxSliders[0] =  10.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// scale
	{
		OP_NumericParameter	np;

		np.name = "Scale";
		np.label = "Scale";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = -10.0;
		np.maxSliders[0] =  10.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// shape
	{
		OP_StringParameter	sp;

		sp.name = "Shape";
		sp.label = "Shape";

		sp.defaultValue = "Sine";

		const char *names[] = { "Sine", "Square", "Ramp" };
		const char *labels[] = { "Sine", "Square", "Ramp" };

		OP_ParAppendResult res = manager->appendMenu(sp, 3, names, labels);
		assert(res == OP_ParAppendResult::Success);
	}

	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";
		
		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

}

void 
MatrixCHOP::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		myOffset = 0.0;
	}
}

