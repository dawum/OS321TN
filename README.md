# OS321TN
git clone https://github.com/dawum/OS321TN.git // cloning git repo

git checkout -b branchname // create and change to that branch

git status // check status of branch

git branch -D branchname // delete branch 

git branch // tells what branches there are 

git checkout filename // to discard changes made on file

git commit file1 file2 or git commit -a // multiple file and commiting all

git commit -m message 

git log --branches --not --remotes // tells what commits not pushed upstream.

git push origin branchname // pushes commits

git config --global --edit // edit config file

AFTER PUSH 
git checkout master 
git pull origin master
git branch -d branchname or git branch -D branchname // to force delete branch
