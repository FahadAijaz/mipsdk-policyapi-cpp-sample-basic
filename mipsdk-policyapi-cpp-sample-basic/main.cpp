﻿/**
*
* Copyright (c) Microsoft Corporation.
* All rights reserved.
*
* This code is licensed under the MIT License.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files(the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "action.h"
#include "mip/common_types.h"
#include "utils.h"
#include "execution_state_impl.h"
#include "mip/upe/metadata_action.h"
#include "mip/upe/protect_by_template_action.h"
#include "mip/upe/justify_action.h"


using std::make_shared;
using std::shared_ptr;
using std::string;

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::vector;

using sample::policy::Action;

int main()
{
	std::string newLabelId;
	std::string currentLabelId;
	std::string clientId = "YOUR CLIENT ID";
	std::string appName = "YOUR APP NAME";
	std::string appVersion = "YOUR APP VERSION";
	std::string userName = "YOUR TEST USER";
	std::string password = "YOUR TEST USER PASSWORD";
		

	// Create the mip::ApplicationInfo object. 
	// Client ID should be the client ID registered in Azure AD for your custom application.
	// Friendly Name should be the name of the application as it should appear in reports.
	mip::ApplicationInfo appInfo{clientId, appName, appVersion };

	// All actions for this tutorial project are implemented in samples::policy::Action
	// Source files are Action.h/cpp.	
	// Action's constructor takes in the mip::ApplicationInfo object and uses the client ID for auth.
	// Username and password are required in this sample as the oauth2 token is obtained via Python script and basic auth.
	Action action = Action(appInfo, userName, password, true);

	// Call action.ListLabels() to display all available labels, then pause.
	action.ListLabels();
	system("pause");

	// This label ID builds the initial execution state, simulating an existing label.
	cout << endl << endl << "Enter a label ID: ";
	cin >> currentLabelId;

	// This label ID builds the new execution state, simulating an updated label.
	cout << endl << "Enter a new label ID: ";
	cin >> newLabelId;

	// Set execution state options and provide to ComputeActions. 
	sample::policy::ExecutionStateOptions options;

	// Build execution state for "current label"
	// This will be used to get metadata to feed to ComputeActions() function to simulate a label change.
	options.newLabel = action.GetLabelById(currentLabelId);
	options.actionSource = mip::ActionSource::MANUAL;
	options.assignmentMethod = mip::AssignmentMethod::STANDARD;
	options.contentFormat = mip::ContentFormat::DEFAULT;
	options.contentIdentifier = "MyTestFile.pptx";
	options.dataState = mip::DataState::USE;	
	options.isDowngradeJustified = false;
	options.generateAuditEvent = true;
		
	// Compute Actions from the provided execution state
	auto initialActions = action.ComputeAction(options);

	// Fetch METADATA action, parse metadata, add to execution state.
	for (const auto action : initialActions)
	{
		switch (action->GetType())
		{
		case mip::ActionType::METADATA:
		{
			options.metadata.clear();
			auto derivedAction = static_cast<mip::MetadataAction*>(action.get());
			for (const std::pair<std::string, std::string>& prop : derivedAction->GetMetadataToAdd())
			{
				options.metadata[prop.first] = prop.second;				
			}
			break;
		}

		case mip::ActionType::PROTECT_BY_TEMPLATE: {
			auto derivedAction = static_cast<mip::ProtectByTemplateAction*>(action.get());
			options.templateId = derivedAction->GetTemplateId();			
			break;
		}
		default:
		{

		}
		}
	}

	// Update execution state to apply the new label.
	options.newLabel = action.GetLabelById(newLabelId);
	
	// Provide desired execution state 
	auto result = action.ComputeActionLoop(options);	
	
	system("pause");

	return 0;
}





