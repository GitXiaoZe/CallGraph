# Instructin :

   ### preprocess.c : 
    used to extract commit id from git log
   
   ### process.cpp : <br>
    convertTree2String() : convert a call graph generated from cflow to a string
    TraversalFile() : Traverse all file recursively
    LCS() : calculate 2 strings' Longest common subsequence    
         


# Dependencies :

    sudo apt-get install cflow g++ git -y
  
# Build :

    use g++ to complie the .c/.cpp file
    
# Usage:

    ./main <directory> <output_path> <commit_id_file> <num>
    or
    ./main <directory> <output_path> <tag_file> <num>
