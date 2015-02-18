// Copyright (c) 2014-2015, THUNDERBEAST GAMES LLC All rights reserved
// Please see LICENSE.md in repository root for license information
// https://github.com/AtomicGameEngine/AtomicGameEngine

#include "AtomicEditor.h"
#include <Atomic/IO/FileSystem.h>
#include <Atomic/IO/File.h>
#include "../AEEditor.h"
#include "../Project/AEProject.h"

#include "../Project/ProjectUtils.h"

#include "BuildSystem.h"
#include "BuildWeb.h"

namespace AtomicEditor
{

BuildWeb::BuildWeb(Context * context) : BuildBase(context)
{

}

BuildWeb::~BuildWeb()
{

}

void BuildWeb::Initialize()
{
    Editor* editor = GetSubsystem<Editor>();
    Project* project = editor->GetProject();

    FileSystem* fileSystem = GetSubsystem<FileSystem>();
    String bundleResources = fileSystem->GetAppBundleResourceFolder();

    String projectResources = project->GetResourcePath();
    String dataFolder = bundleResources + "Data/";
    String coreDataFolder = bundleResources + "CoreData/";

    AddResourceDir(coreDataFolder);
    AddResourceDir(dataFolder);
    AddResourceDir(projectResources);

    BuildResourceEntries();
}
void BuildWeb::Build(const String& buildPath)
{
    buildPath_ = buildPath;

    Initialize();

    FileSystem* fileSystem = GetSubsystem<FileSystem>();
    String buildSourceDir = fileSystem->GetAppBundleResourceFolder();
    buildSourceDir += "Deployment/Web";

    fileSystem->CreateDir(buildPath);

    String resourcePackagePath = buildPath + "/AtomicResources.data";
    GenerateResourcePackage(resourcePackagePath);

    fileSystem->Copy(buildSourceDir + "/AtomicPlayer.html", buildPath + "/AtomicPlayer.html");
    fileSystem->Copy(buildSourceDir + "/AtomicPlayer.js", buildPath + "/AtomicPlayer.js");

    File file(context_, buildSourceDir + "/AtomicResources_js.template", FILE_READ);
    unsigned size = file.GetSize();

    SharedArrayPtr<char> buffer(new char[size + 1]);
    file.Read(buffer.Get(), size);
    buffer[size] = '\0';

    String resourcejs = (const char*) buffer.Get();

    file.Close();

    file.Open(buildPath + "/AtomicResources.data", FILE_READ);
    int rsize = (int) file.GetSize();
    file.Close();

    String request;
    request.AppendWithFormat("new DataRequest(0, %i, 0, 0).open('GET', '/AtomicResources.pak');", rsize);

    resourcejs.Replace("$$ATOMIC_RESOURCES_DATA_REQUEST$$", request);

    file.Open(buildPath + "/AtomicResources.js", FILE_WRITE);
    file.Write(resourcejs.CString(), resourcejs.Length());
    file.Close();

    //ProjectUtils* utils = GetSubsystem<ProjectUtils>();
    //utils->RevealInFinder(GetPath(buildPath));

    BuildSystem* buildSystem = GetSubsystem<BuildSystem>();
    buildSystem->BuildComplete();

    //fileSystem->SystemCommandAsync("/Applications/Firefox.app/Contents/MacOS/firefox");

}

}
