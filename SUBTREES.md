# Subtrees

Modules from other git repositories are used within this project
via the subtree functionality of git.

For easier access the repository is add as remote
```
git remote add -f mkcres https://github.com/jahnf/mkcres
```
```
git remote add -f strawberry-ini https://github.com/jahnf/strawberry-ini
```

Add the subtree was done with this command (`--squash` ignores the history)
```
git subtree add --prefix mkcres mkcres master --squash
```
```
git subtree add --prefix strawberry-ini strawberry-ini master --squash
```

The command to update the sub-project at a later date becomes:
```
git fetch mkcres master
git subtree pull --prefix mkcres mkcres master --squash
```
```
git fetch strawberry-ini master
git subtree pull --prefix strawberry-ini strawberry-ini master --squash
```

> **Cygwin:** git-subtree is not part of a package. 
> However, like gitk you can manually install it
```
wget rawgit.com/git/git/master/contrib/subtree/git-subtree.sh
install git-subtree.sh /bin/git-subtree
```