/**
    Theme: Homework corrention helper
    compiler: g++ 4.7.1 (C::B on win8)
    Date: 103/01/12
    Author: Yans.
    Ref.: http://cg2010studio.wordpress.com/
*/
//{ =====includes=====
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
using namespace std;
//} ==================

string getFilenameExtension(string Filename);//副檔名
bool readFullText2string(ifstream& fin,string& text);
int getdir(string dir, vector<string> &files);
//we can use "addIncludes" by
//#define fin(x) fin("the_file_you_want")
//to change fin files
int addIncludes(ifstream& fin_toModify,ifstream& fin_refMain,ofstream& fout);
int changeMain(ifstream& fin_toModify,ifstream& fin_refMain,ofstream& fout);
inline bool inside(double x,double min,double max){return x>=min && x<=max;}

class ProblemBR{//build and run
public:
    // must
    string thisHomeworkDir;//絕對位址  ex."D:\\yansA-TA\\HWCorrecting\\HW01";
    string compilerDirPath;//絕對位址  ex."C:\\Program Files (x86)\\CodeBlocks\\MinGW\\bin"
    bool lots_of_patterns; //若測資檔超多，lots_of_patterns設為true。則視testPatternPath為資料夾(裡面全都是測資檔)
    string testPatternPath;//絕對位址  ex."D:\\yansA-TA\\HWCorrecting\\mid_ans\\correcting\\pat_p6"
    // build related
    string compiler;// = "g++";
    string thisProNameFeature;
    string logFileName;
    vector<string> searchPaths;
    vector<string> staticLibraries;
    // optional
    string changeMainFile;
    string addIncludeFile;
private:
    vector<string> thePatterns;
    vector<string> studentsFolders;
    //{ -----error checking-----
    bool isOK;
    string errorLog;
    //} ------------------------

    string getStuID(string raw){
        size_t found;
        found = raw.find("b02901");
        if(found!=string::npos){ raw = raw.substr(  found,9  );  return raw;  }
        found = raw.find("b01502");
        if(found!=string::npos){ raw = raw.substr(  found,9  );  return raw;  }
        return raw = "unknow";
    }
    void updateStuInfo(const int stu_i,string& studentDir,vector<string>& studentFiles){
        studentDir = thisHomeworkDir+"\\"+studentsFolders[stu_i];
        getdir(studentDir, studentFiles);
    }
    bool get_stuProblemTarget(const vector<string>& studentFiles,string& theProblemTarget){
        //for one student to search one's folder
        size_t found;
        for(int i=0; i<studentFiles.size(); i++){
            found = studentFiles[i].find(thisProNameFeature);
            if(found!=string::npos){
                found = i;

                theProblemTarget = studentFiles[found];
                string type = getFilenameExtension(theProblemTarget);
                if(type=="txt" || type=="exe") continue;
                break;
            }
        }
        if(found==string::npos){
            //cout<< studentsFolders[stu_i]<<"can not find "+thisProNameFeature+" file!!"<< endl;
            theProblemTarget = "";
            return false;//system("pause");
        }else{
            theProblemTarget = studentFiles[found];
            return true;
        }
    }
    bool add_include(const string& studentDir,string& theCppFile){
        if(addIncludeFile.length()!=0){
            ifstream fin((studentDir+"\\"+theCppFile).c_str());
            ifstream fin_ref(addIncludeFile.c_str());
            ofstream addincout((studentDir+"\\"+theCppFile+"_ainc.cpp").c_str());

            int check = addIncludes(fin,fin_ref,addincout);
            fin.close();fin_ref.close();addincout.close();

            if(check==0){//safe
                theCppFile = theCppFile+"_ainc.cpp";
                return true;
            }else{
                cout<< studentDir+"\\"+theCppFile <<" add_include fail<check = "<< check <<">!!"<< endl;
                return false;
            }
        }return true;
    }
    bool change_main(const string& studentDir,string& theCppFile){
        if(changeMainFile.length()!=0){
            ifstream fin((studentDir+"\\"+theCppFile).c_str());
            ifstream fin_ref(changeMainFile.c_str());
            ofstream xmainout((studentDir+"\\"+theCppFile+"_xmain.cpp").c_str());

            int check = changeMain(fin,fin_ref,xmainout);
            fin.close();fin_ref.close();xmainout.close();

            if(check==0){//safe
                theCppFile = theCppFile+"_xmain.cpp";
                return true;
            }else{
                cout<< studentDir+"\\"+theCppFile <<" change_main fail<check = "<< check <<">!!"<< endl;
                return false;
            }

        }return true;
    }
    void compile_student_cpp(const string& studentDir,const string& theCppFile){
        //compile .cpp to .o
        string cmd = compiler+" -c "+studentDir+"\\"+theCppFile+" -o "+studentDir+"\\"+theCppFile+".o";
        if(searchPaths.size()>0){
            cmd+=" -I ";  for(int i=0;i<searchPaths.size();++i) cmd+= (searchPaths[i]+" ");
        }
        system(  cmd.c_str()  );
        //link .o to .exe
        cmd = compiler+" -o "+studentDir+"\\"+theCppFile+".exe "+studentDir+"\\"+theCppFile+".o ";
        if(staticLibraries.size()>0){
            for(int i=0;i<staticLibraries.size();++i) cmd+= (staticLibraries[i]+" ");
        }
        system(  cmd.c_str()  );
    }
    void compile_student_project(const string& studentDir,const string& theProjectDir){
        string projectPath = studentDir+"\\"+theProjectDir;
        vector<string> stuProjectFiles = vector<string>();
        vector<string> objFiles = vector<string>();
        getdir(projectPath, stuProjectFiles);//搜尋資料夾和檔案名稱
        string cmd;
        string filteredName;

        //compile .cpp to .o
        for(int i=0;i<stuProjectFiles.size();++i){
            string type = getFilenameExtension(stuProjectFiles[i]);
            if( type!="cpp" ) continue;
            //{ -----for ainc & xmain-----
            if(stuProjectFiles[i]==filteredName) continue;
            filteredName = stuProjectFiles[i];
            int additionNamePos = -1;
            additionNamePos = filteredName.find("_xmain.cpp");
            if(additionNamePos!=string::npos) filteredName = filteredName.substr(0,additionNamePos);
            additionNamePos = filteredName.find("_ainc.cpp");
            if(additionNamePos!=string::npos) filteredName = filteredName.substr(0,additionNamePos);
            //} --------------------------

            //cmd = compiler+" -c "+studentDir+"\\"+theProjectDir+"\\"+stuProjectFiles[i]+" -o "+studentDir+"\\"+theProjectDir+"\\"+stuProjectFiles[i]+".o ";
            cmd = compiler+" -c "+studentDir+"\\"+theProjectDir+"\\"+stuProjectFiles[i]+" -o "+studentDir+"\\"+theProjectDir+"\\"+filteredName+".o ";
            if(searchPaths.size()>0){
                cmd+=" -I ";  for(int i=0;i<searchPaths.size();++i) cmd+= (searchPaths[i]+" ");
            }
            system(  cmd.c_str()  );
            //objFiles.push_back(studentDir+"\\"+theProjectDir+"\\"+stuProjectFiles[i]+".o");
            if(filteredName==stuProjectFiles[i])
                objFiles.push_back(studentDir+"\\"+theProjectDir+"\\"+filteredName+".o");
        }
        // .a
        for(int i=0;i<stuProjectFiles.size();++i){ if( getFilenameExtension(stuProjectFiles[i])=="a" ) objFiles.push_back(studentDir+"\\"+theProjectDir+"\\"+stuProjectFiles[i]); }
        for(int i=0;i<staticLibraries.size();++i) objFiles.push_back(staticLibraries[i]);

        //link .o to .exe
        cmd = compiler+" -o "+studentDir+"\\"+theProjectDir+".exe ";
        for(int i=0;i<objFiles.size();++i){
            cmd += objFiles[i]+" ";
        }
        system(  cmd.c_str()  );
    }
    string project_find_main(const string& studentDir,const string& theProjectDir){
        string projectPath = studentDir+"\\"+theProjectDir;
        vector<string> stuProjectFiles = vector<string>();
        getdir(projectPath, stuProjectFiles);//搜尋資料夾和檔案名稱

        ifstream fin;
        string text;
        int foundMain = -1;

        for(int i=0;i<stuProjectFiles.size();++i){
            string type = getFilenameExtension(stuProjectFiles[i]);
            if( type!="cpp" ) continue;

            fin.open(   (projectPath+"\\"+stuProjectFiles[i]).c_str()   );
            if( !readFullText2string(fin,text) ) cout<< "file reading error:"+stuProjectFiles[i] << endl;
            else{
                foundMain = text.find("main");
                if(foundMain!=string::npos) return stuProjectFiles[i];
            }
            fin.close();
        }
        cout<< "cannot find .cpp file having main!!"<<endl;
        return "";
    }
    void congratulation(){
        cout<< endl;
        for(double y=5;y>=-3;y-=0.5){
            for(double x=-8;x<=8;x+=0.25){
                if( inside(x,-2.5,-1.0)&&inside(y,0.5,2.5) ) cout<< ' ';
                else if( inside(x,-3.0,-0.5)&&inside(y,0.0,2.5) ) cout<< '@';
                else if( inside(x,1.1,2.0)&&inside(y,0.0,2.0) ) cout<< ' ';
                else if( inside(x,0.6,2.5)&&inside(y,1.4,1.6) ) cout<< ' ';
                else if( inside(x,0.6,2.5)&&inside(y,0.0,2.0) ) cout<< '@';
                else if(  (x*x+3*y*y-30)*(x*x+3*y*y-30)*(x*x+3*y*y-30)-90*x*x*y*y*y <0) cout<< ' ';
                else cout<< '@';
            } cout<< '\n';
        }
    }
public:
    void using_pdcurses(){
        searchPaths.push_back(".\\yans_pdcurses");
        staticLibraries.push_back(".\\yans_pdcurses\\pdcurses.a");
    }
    void init(){
        if(lots_of_patterns)
            getdir(testPatternPath, thePatterns);//搜尋資料夾和檔案名稱
        getdir(thisHomeworkDir, studentsFolders);//搜尋資料夾和檔案名稱
    }
    void build(){
        string studentDir;//絕對位址
        vector<string> studentFiles;
        string theCppFile;  string theProblemTarget;
        system(  ("set path="+compilerDirPath+";%path%").c_str()  );

        for(int i=0; i<studentsFolders.size(); i++){
            if(studentsFolders[i]==".") continue;
            if(studentsFolders[i]=="..") continue;
            //one student folder enterred

            //{ =====update Stu & find target=====
            updateStuInfo(i,studentDir,studentFiles);
            get_stuProblemTarget(studentFiles,theProblemTarget);
            if(theProblemTarget==""){//error
                cout<< studentsFolders[i]<<"can not find "+thisProNameFeature+" file!!"<< endl;
                isOK = false;
            }else isOK = true;
            string type = getFilenameExtension(theProblemTarget);
            //} ==================================
            if(!isOK){ system("pause"); studentFiles.clear(); continue; }
            //{ =====pre-cppfixing=====
            if(type=="cpp"){//single file
                isOK = add_include(studentDir,theProblemTarget) && change_main(studentDir,theProblemTarget);
            }else if(type==""){//project folder
                string theMainFile = project_find_main(studentDir,theProblemTarget);
                if(theMainFile.length()!=0){
                    theMainFile = theProblemTarget+"\\"+theMainFile;
                    isOK = add_include(studentDir,theMainFile);
                    isOK = isOK&&change_main(studentDir,theMainFile);
                }
            }else{//not support
                cout<< studentsFolders[i]<<" format of "+thisProNameFeature+" not support!!<"+theProblemTarget+">!!"<< endl;
                isOK = false;
            }
            //} =======================
            if(!isOK){ system("pause"); studentFiles.clear(); continue; }
            //{ =====compile student target=====
            cout<<"compiling "<<studentsFolders[i]+"/"+theProblemTarget<<endl;
            if(type=="cpp"){//single file
                compile_student_cpp(studentDir,theProblemTarget);
            }else{//project folder
                compile_student_project(studentDir,theProblemTarget);
            }
            //} ================================
            studentFiles.clear();
        }
    }
    void generateBat(string batName){
        ofstream fout(batName.c_str());//"runAll.bat"

        string studentDir;//絕對位址
        vector<string> studentFiles;
        string theProblemTarget;
        string target_exePath;
        for(int i=0; i<studentsFolders.size(); i++){
            if(studentsFolders[i]==".") continue;
            if(studentsFolders[i]=="..") continue;
            //one student folder enterred

            //{ =====update Stu & find target=====
            updateStuInfo(i,studentDir,studentFiles);
            get_stuProblemTarget(studentFiles,theProblemTarget);
            string type = getFilenameExtension(theProblemTarget);
            //} ==================================
            if(theProblemTarget==""){ studentFiles.clear(); continue; }
            //{ =====pre-cppfixing=====
            if(type=="cpp"){//single file
                if(addIncludeFile.length()!=0)
                    theProblemTarget = theProblemTarget+"_ainc.cpp";
                if(changeMainFile.length()!=0)
                    theProblemTarget = theProblemTarget+"_xmain.cpp";
            }else if(type==""){//project folder

            }else{ studentFiles.clear(); continue; }
            //} =======================
            target_exePath = studentDir+"\\"+theProblemTarget+".exe";
            //{ =====Gen bat file=====
            if(lots_of_patterns){
                fout<< "echo start > "+studentDir+"\\"+logFileName<<endl;
                fout<< "    cd "+studentDir<< endl;
                for(int i=0;i<thePatterns.size();++i){
                    if(thePatterns[i]==".") continue;
                    if(thePatterns[i]=="..") continue;
                    fout<< "    "+theProblemTarget+".exe"+" < "+testPatternPath+"\\"+thePatterns[i]+" >> "+logFileName<<endl;//append
                }
            }else
                fout<< target_exePath+" < "+testPatternPath+" > "+studentDir+"\\"+logFileName<<endl;
            //} ======================
            studentFiles.clear();
        }
        fout<< "pause\n";
        fout.close();
        system("pause");congratulation();
    }
    void directCompare(string batName,string goldenLogPath){//auto compare log(gen by bat) and goldenLog
        ifstream goldenLog_fin((goldenLogPath).c_str());
        string golden;
        readFullText2string(goldenLog_fin,golden);
        goldenLog_fin.close();

        ifstream bat_fin((batName).c_str());
        ifstream log_fin;
        char lineBuf[1024];string temp;
        string logFName;string logContent;
        bool pass; int charDiff;
        //run
        while(  bat_fin.getline(lineBuf,1024)  ){
            temp = lineBuf;
            if(temp.find("pause")!=string::npos) break;
            cout<<"running "+getStuID(temp)+" exe" <<endl;
            system(lineBuf);//run
            if(temp.find("echo")!=string::npos) continue;
        }bat_fin.close();
        //read again
        bat_fin.open((batName).c_str());
        while(  bat_fin.getline(lineBuf,1024)  ){
            temp = lineBuf;
            if(temp.find("pause")!=string::npos) break;
            if(temp.find("echo")!=string::npos) continue;

            temp = temp.substr(  temp.rfind(">")+2  );
            if(temp!=logFName){//not lots_of_patterns case
                logFName = temp;
                log_fin.open(logFName.c_str());
                logContent.clear();
                if(  readFullText2string(log_fin,logContent)  ){
                    pass=true;charDiff=0;
                    for(int i=0;i<golden.length();++i){
                        if(logContent[i]=='\0'){pass=false;break;}
                        if(logContent[i]!=golden[i]){
                            pass=false;++charDiff;
                        }
                    }
                    if(pass) cout<< getStuID(logFName)+" pass direct compare"<< endl;
                    else     cout<< getStuID(logFName)+" fail with "<< charDiff << "char different, length%<"<< logContent.length()<<"/"<< golden.length()<<"> "<< endl;
                }else cout<< getStuID(logFName)+" fail : no log"<< endl;
                log_fin.close();
            }
        }
        bat_fin.close();
    }
};


int main(){
    //{ =====global setting=====
    ProblemBR corrector;
    corrector.thisHomeworkDir = "D:\\yansA-TA\\Introduction_to_Computer\\HWCorrecting\\HW01";//絕對位址
    corrector.compilerDirPath = "C:\\Program Files (x86)\\CodeBlocks\\MinGW\\bin";//絕對位址
    corrector.testPatternPath = "D:\\yansA-TA\\Introduction_to_Computer\\HWCorrecting\\AutoCorrecting\\HW01_ans\\pat";//絕對位址
    //若測資檔超多，lots_of_patterns設為true。則視testPatternPath為資料夾(裡面全都是測資檔)
    corrector.lots_of_patterns = true;

    corrector.changeMainFile = "D:\\yansA-TA\\Introduction_to_Computer\\HWCorrecting\\AutoCorrecting\\HW01_ans\\main_ch.txt";//絕對位址
    //if no need to change main, use ""
    corrector.addIncludeFile = "D:\\yansA-TA\\Introduction_to_Computer\\HWCorrecting\\AutoCorrecting\\HW01_ans\\include.txt";//絕對位址
    //if no need to change main, use ""

    corrector.compiler = "g++";
    corrector.thisProNameFeature = "HW";
    corrector.logFileName = "log.txt";

    //corrector.using_pdcurses();
    //} ========================

    corrector.init();
    corrector.build();
    corrector.generateBat("runAll.bat");
    //corrector.directCompare("runAll.bat","D:\\yansA-TA\\HWCorrecting\\HW03_ans\\HW03_p4_db03_golden.txt");

    return 0;
}
string getFilenameExtension(string Filename){
    unsigned found = Filename.find_last_of(".");
    string type;
    if(found==string::npos) return type;
    else return (  type = Filename.substr(found+1)  );
}
int getdir(string dir, vector<string> &files){
    DIR *dp;//創立資料夾指標
    struct dirent *dirp;
    if((dp = opendir(dir.c_str())) == NULL){
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }
    while((dirp = readdir(dp)) != NULL){//如果dirent指標非空
        files.push_back(string(dirp->d_name));//將資料夾和檔案名放入vector
    }
    closedir(dp);//關閉資料夾指標
    return 0;
}
bool readFullText2string(ifstream& fin,string& text){
    if(!fin){
        cout<< "error to read file!!"<<endl;
        return false;
    }
    bool star_comment = false;
    size_t found;
    char lineBuf[512];
    string line;
    do{
        fin.getline(lineBuf,512);
        line = lineBuf;
        line = line.substr(  0,line.find("//")  );

        if(!star_comment){
            found = line.find("/*");
            if(found!=string::npos){
                size_t found2 = line.find("*/");
                if(found2!=string::npos){
                    string head = line.substr(  0,found  );
                    string tail = line.substr(  found2+2  );
                    line = head+tail;
                }else{
                    line = line.substr(  0,found  );
                    star_comment = true;
                }
            }
        }else{
            found = line.find("*/");
            if(found==string::npos){
                line="";
            }else{
                if(line.length()>=found+2)
                    line = line.substr(  found+2  );
                else
                    line="";
                star_comment = false;
            }
        }

        text += line+'\n';
    }while(fin);
//cout<<"--\n"
//    <<text<<"\n--\n";
    return true;
}
int addIncludes(ifstream& fin_toModify,ifstream& fin_refMain,ofstream& fout){
    string planeText;
    if(!readFullText2string(fin_toModify,planeText)){
        cout<< "read file error"<<endl;
        return -1;
    }
    string refText;
    readFullText2string(fin_refMain,refText);

    planeText = refText+planeText;

    fout.write(planeText.c_str(),planeText.length());
    return 0;
}
int changeMain(ifstream& fin_toModify,ifstream& fin_refMain,ofstream& fout){
    string planeText;
    if(!readFullText2string(fin_toModify,planeText)){
        cout<< "read file error"<<endl;
        return -1;
    }

    int found = -1,mainCount=-1;
    do{
        found = planeText.find("main",found+1);
        ++mainCount;
    }while(found!=string::npos);

    if(mainCount>1){
        cout<< "there are " << mainCount << " \"main\" in this file!" << endl;
        fout.write(planeText.c_str(),planeText.length());
        return -2;
    }

    string refText;
    if(mainCount==1){
        size_t mainStart = planeText.find("main");
        size_t mainEnd;
        found = planeText.find("{",mainStart+1);
        int braceCount = 0;
        for(int i=found,len=planeText.length();i<len;++i){
            if(planeText[i]=='{') ++braceCount;
            else if(planeText[i]=='}') --braceCount;

            if(braceCount==0){
                mainEnd = i;
                break;
            }else continue;
        }
        if(braceCount!=0){
            cout<< "brace error, there might be syntex error in the cpp file!"<< endl;
            return -3;
        }
        readFullText2string(fin_refMain,refText);
        planeText.replace(mainStart,mainEnd-mainStart+1,refText);
    }else{//mainCount==0
        readFullText2string(fin_refMain,refText);
        planeText.append(refText);
    }

    fout.write(planeText.c_str(),planeText.length());
    return 0;
}


