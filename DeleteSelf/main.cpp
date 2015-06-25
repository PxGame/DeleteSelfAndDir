# include <afxwin.h>
# include <ShlObj.h>
# include <Shlwapi.h>

void DeleteDirectory(CString strDir)
{
    if (strDir.IsEmpty())
    {
        //RemoveDirectory(strDir);
        return;
    }

    //首先删除文件及子文件夹 
    CFileFind   ff;
    BOOL bFound = ff.FindFile(strDir + _T("\\*"), 0);

    while (bFound)
    {
        bFound = ff.FindNextFile();
        CString fileName = ff.GetFileName();
        CString filePath = ff.GetFilePath();
        if (ff.GetFileName() == _T(".") || ff.GetFileName() == _T(".."))
            continue;

        //去掉文件(夹)只读等属性 
        SetFileAttributes(ff.GetFilePath(), FILE_ATTRIBUTE_NORMAL);

        if (ff.IsDirectory())
        {
            //递归删除子文件夹 
            DeleteDirectory(ff.GetFilePath());
        }
        else
        {
            DeleteFile(ff.GetFilePath());   //删除文件
        }
    }

    ff.Close();

    //然后删除该文件夹 
    RemoveDirectory(strDir);
}

bool SelfDelete()
{
    const TCHAR	COMSPEC[] = TEXT("COMSPEC");
    const TCHAR	PARA_CMD[] = TEXT(" /c del ");
    const TCHAR	RE_DIRECTION[] = TEXT(" >NUL");
    TCHAR		szCommand[MAX_PATH] = { 0 };
    TCHAR		szModuleName[MAX_PATH] = { 0 };
    HANDLE		hProcess;
    HANDLE		hThread;
    STARTUPINFO			si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    GetEnvironmentVariable(COMSPEC, szCommand, sizeof(szCommand));
    GetModuleFileName(0, szModuleName, sizeof(szModuleName));
    GetShortPathName(szModuleName, szModuleName, sizeof(szModuleName));

    lstrcat(szCommand, PARA_CMD);
    lstrcat(szCommand, szModuleName);
    lstrcat(szCommand, RE_DIRECTION);

    hProcess = GetCurrentProcess();
    SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
    if (!SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS)) return false;
    hThread = GetCurrentThread();
    if (SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL))
    {
        if (CreateProcess(NULL, szCommand, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        {
            SetPriorityClass(pi.hProcess, IDLE_PRIORITY_CLASS);
            SetProcessPriorityBoost(pi.hProcess, TRUE); // 临时提升指定进程中的线程的优先级
            //通知系统：应用程序已完成的事件。当应用程序会影响外壳的时候，调用此函数
            SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, szModuleName, NULL);
            return true;
        }
    }
    else SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);
    return false;
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
    TCHAR dirPath[MAX_PATH] = { 0 };
    GetModuleFileName(0, dirPath, sizeof(dirPath));
    PathRemoveFileSpec(dirPath);
    DeleteDirectory(dirPath);

    SelfDelete();
    return 0;
}