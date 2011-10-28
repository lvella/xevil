/* 
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 * satan@xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, the file "gpl.txt"; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA, or visit http://www.gnu.org.
 */

#ifndef _FILEMAN_H
#define _FILEMAN_H


class TempFileManager
{
    char *directoryname; //stores the temp directory name 
    char **tempfiles;    //array of temporary files
    int numtempfiles;    //number of temporary files
    int buffersize;      //size of buffer for temp file array
    void addName(char *);//private method that adds a name to the temp file array
public:
    TempFileManager(); 
    ~TempFileManager();

    const char *newTempFileName(const char *ext); //returns a new name prepended with ~xtmp

    int getNumTempFiles(){return numtempfiles;} //returns number of temporary files
    const char *getTempFileByIndex(int index);  //returns filename from the index into the array
    void cleanup();                             //deletes the files from the temp file list.

    /*attachDirecetory goes through passed in directory and adds all appropriate files to the 
      temp file array */
    void attachDirectory(const char *dirname);  
};

#endif //_FILEMAN_H
