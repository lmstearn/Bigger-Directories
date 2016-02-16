bool fsDelsub (int i, int j, HWND hwnd)
{

errorCode = 0;
		

	findPathW[0] = L'\0'; // driveIDBaseWNT L"C:\\"

	wcscpy_s(findPathW, maxPathFolder, driveIDBaseWNT);
	wcscat_s(findPathW, maxPathFolder, pathsToSave[j]);


	for (k = trackFTA [i][1]; (k >= ((i < branchTotalDel)? trackFTA [i + 1][1]: 0)); k--)
	{
	//do not iterate below trackFTA [i + 1][1]



	if (RemoveDirectoryW (findPathW))
			{
				//clear everything
				pathsToSave[j][0] = L'\0';
				//we have only removed the last dir from pathsToSave so remove last dir from folderTreeArray
							

				folderTreeArray[j][k-1][0] = L'\0';

				trackFTA [i][1] -=1;
				trackFTA [j][0] -=1;


				if (trackFTA [i][1] != 0)
				{
				//rebuild pathsToSave
				for (int l = 0; (l < k - 1); l++) //extra loop adds the terminator
				{
					if (l != 0) wcscat_s(pathsToSave[j], maxPathFolder, &separatorFTA);
					wcscat_s(pathsToSave[j], maxPathFolder, folderTreeArray[j][l]);

				}
				findPathW[0] = L'\0'; // driveIDBaseWNT L"C:\\"

				wcscpy_s(findPathW, maxPathFolder, driveIDBaseWNT);
				wcscat_s(findPathW, maxPathFolder, pathsToSave[j]);
				return true;
				}
				else
				{
				goto FSReorg;
				}
			
			}
		else
			{
				if (((int)GetLastError() == 32) ) //"used by another process" error
				{
					memset(rootDir, L'\0', sizeof(rootDir));
					//wcstombs (pCmdLine, findPathW, maxPathFolder);
					pCmdLineActive = true;
					wcscpy_s(findPathW, maxPathFolder, L" "); //http://forums.codeguru.com/showthread.php?213443-How-to-pass-command-line-arguments-when-using-CreateProcess
					wcscat_s(findPathW, maxPathFolder, rootDir);
					((ProcessfileSystem(hwnd, true, false))? errorCode = 0: errorCode = 1);
					Kleenup (hwnd, weareatBoot);
					return false;

				}
				else
				{
					if (((int)GetLastError() == 2) || ((int)GetLastError() == 3)) //cannot find file or path specified
						{
						//The entry in pathsToSave must have a duplicate elsewhere: nuke the current one:
						pathsToSave[j][0] = L'\0';
								
						folderTreeArray[j][0][0] = L'\0';

						trackFTA [i][1] = 0;
						trackFTA [j][0] = 0;
						goto FSReorg;
						}
					else
						{
						ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);
						return false;
					}

				}
				//No rollback

			}

	}
	//k loop

return true;

FSReorg:
if (j == branchTotalDel) return true;
//Move everything down to fill slot
for (k = j + 1; (k <= branchTotalDel); k++)
{
	for (int l = 0; (l < trackFTA [k][0]); l++)
		{
		wcscpy_s(folderTreeArray[k-1][l], maxPathFolder, folderTreeArray[k][l]); 
		}	

	trackFTA [k-1][0] = trackFTA [k][0];
	wcscpy_s(pathsToSave[k-1], maxPathFolder, pathsToSave[k]); 
}

branchTotalDel -= 1;
return true;
}
