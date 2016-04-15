# Introduction #

Since [R18](https://code.google.com/p/sa-mp-mysql-plugin/source/detail?r=18) cache\_save-functions are available. But what exactely are they doing?
With these functions you can save the currently active cache in memory for later uses. Well, doesn't really sound easy, but in fact it's not that hard.


# Example #

```
public OnPlayerConnect(playerid) {
    //Other code...
    new Query[128];
    format(Query, sizeof(Query), "SELECT * FROM Players WHERE Name = '%s'", PlayerName);
    
    //send a query to retrieve user information, nothing new
    mysql_function_query(SQL, Query , true, "OnPlayerDataLoaded", "i", playerid);
}


new StoredPlayerCache[MAX_PLAYERS];

public OnPlayerDataLoaded(playerid) {
    //Now the result of the query is in the cache. 
    //In this public the active cache now holds the retrieved user information.
    //But we don't want to assign it to user variables yet. Now thats where the new functions are used.

    StoredPlayerCache[playerid] = cache_save();
    //when cache_save is called, the active cache (in this case the cache with the user information stored) is 
    //saved permanently in memory. cache_save returns an ID for later use.
    
    //Now lets make a timer that will assign the retrieved user information to player variables.
    SetTimerEx(10000, false, "SetPlayerData", "d", playerid);
}


public SetPlayerData(playerid) {
    //now we are in another public, where no query or other has been sent before, so there is currently no active cache.
    //but we saved the cache before and now we can set the saved cache as the active cache.

    cache_set_active(StoredPlayerCache[playerid]);

    //now the active cache is set. If there is an active cache, we can use all other cache functions as normal.

    new Rows, Fields;
    cache_get_data(Rows, Fields);
    printf("There are %d fields in the result set.", Fields);
    if(Rows) {
        PlayerInfo[playerid][Money] = cache_get_field_content_int(0, "Money");
        //other assignments...
    }

    //if you are done using the cache and don't want it to be active anymore, use cache_delete
    cache_delete(StoredPlayerCache[playerid]);
    StoredPlayerCache[playerid] = 0;
}
```

# Important information #

  * you don't have to use this functions
  * when saved cache is set as active, the new active cache will remain active until it is deleted or another query overwrites it (don't worry, this won't cause any memory leaks)
  * there will be no memory leaks if you don't delete the cache (but it's better if you do it)
  * if trying to save an already saved cache, it will return the ID of the already saved one (so the cache won't be saved again)
  * when storing the active cache, the active cache won't be invalidated, so you can continue using the cache functions in the current public