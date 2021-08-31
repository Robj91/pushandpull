//for the purpose of streamlining, this script contains the code that is called in any moveable sprite script
//instead of copy pasting bulk loads of code into every sprites script that I want to add this system into.

void main(void)
{
 //preload push sequence
 preload_seq(312);
 preload_seq(314);
 preload_seq(316);
 preload_seq(318);

 //make sure this sprite has a touch damage of -1, and it's hard.
 sp_touch_damage(&current_sprite, -1);
 sp_hard(&current_sprite, 0);
 
 //make sure the rock has a speed value
 &save_x = sp_speed(&current_sprite, -1);
 if (&save_x <= 0)
 {
  //player forgot to set a speed! we'll give it one..
  sp_speed(&current_sprite, 1);
 }
 
 //save the speed and timing of the sprite in case we need to change it
 &save_x = sp_speed(&current_sprite, -1);
 &save_y = sp_timing(&current_sprite, -1);
 sp_custom("oldspeed", &current_sprite, &save_x);
 sp_custom("oldtiming", &current_sprite, &save_y); 
 
 //we'll update map hardness just in case hardness wasn't set
 draw_hard_sprite(&current_sprite); 

 //if any of the &arg 1 - 4 vars are not equal to 0, the player has probably provided hardbox values. 
 //It's possible for one hardbox value to be 0.
 //So don't make the (!= 0) condition mandatory for &arg1 - 4.
 //If any of these return something other than 0, the player has attempted to pass hardbox values
 //If they didn't pass the properly, or missed one, that's their problem to fix.
 &save_x = 0;
 if (&arg1 != 0)
  &save_x = 1;
 
 if (&arg2 != 0)
  &save_x = 1;

 if (&arg3 != 0)
  &save_x = 1;
  
 if (&arg4 != 0)
  &save_x = 1;

 if (&save_x == 1)
 {
  //Let future scripts know that hardbox boundaries have been provided, so no need for automatic hardbox detection. Yay for no lag!
  sp_custom("HBOX", &current_sprite, 1);
 
  //let's save the hardbox boundaries for later use
  sp_custom("LEFT-BOX", &current_sprite, &arg1);
  sp_custom("TOP-BOX", &current_sprite, &arg2);
  sp_custom("RIGHT-BOX", &current_sprite, &arg3);
  sp_custom("BOTTOM-BOX", &current_sprite, &arg4);

  //Check whether to allow collision system or not
  &save_x = sp_custom("setcollision", &current_sprite, -1);
  if (&save_x > 0)
  {
   //allow collision
   sp_custom("AllowCollision", &current_sprite, 1);
  }
 }
 
 kill_this_task();
}

void touch(void)
{ 
 //manual termination has been called by author and push/pull has not been re-initiated. Do not proceed.
 &save_x = sp_custom("terminated", &current_sprite, -1);
 if (&save_x > 0)
  return;

 sp_touch_damage(&current_sprite, 0);

   //create a shadow sprite so we can easily retrieve it without messing with other sprites or using a global
   //Could use a custom key, but we want something that we can find with the least amount of looping through sprites.
   //We save the active sprite number of the retrievable shadow sprite in a custom key attached to &current_sprite
   //That way we can cross reference it - to avoid some rare bugs.
   &save_x = sp_custom("PP-Shadow", &current_sprite, -1);
   &save_y = sp_custom("PP-Parent", &save_x, -1);
   if (&save_y != &current_sprite)
   {
    //the shadow sprite doesn't currently exist, so we can go ahead and create one.
    &save_x = sp_pseq(&current_sprite, -1);
    &save_y = sp_pframe(&current_sprite, -1);
    &save_x = create_sprite(1, 1, 15, &save_x, &save_y);
    sp_brain(&save_x, 15);
    sp_brain_parm(&save_x, &current_sprite);
    sp_nodraw(&save_x, 1);
    sp_nohit(&save_x, 1);
    sp_custom("PP-Shadow", &current_sprite, &save_x);
    
    //also save this sprite in a custom key attached to the shadow sprite.
    //this allows us to cross reference. Just relying on retrieving "sp_brain_parm" of the shadow can cause bugs.
    //If the developer has attached more than 1 shadow to their moveable sprite.
    //Because if the shadow dies before "PP-Shadow" is reset, another shadow might take the same active sprite #
    //And if that gets detected as the "PP-Shadow" sprite, things will mess up.
    sp_custom("PP-Parent", &save_x, &current_sprite);
   }
   else
   {
    //shadow sprite already exists
    //somehow this has run twice, we don't want that, kill it off.
    //this shouldn't even really happen, but I'll leave it here for a fail safe just in case.
    kill_this_task();
   } 

   //check if an instance of pup.c already exists(spawned by another sprite) and if it does, use that to save resources
   &save_x = 0;
  findpup:
   &save_x = get_next_sprite_with_this_brain(15, 0, &save_x);
   if (&save_x > 0)
   {
    &save_y = sp_custom("PP-pup", &save_x, -1);
    if (&save_y > 0)
    {
     if (&save_y == 0)
     {
      //Here we have determined that Dink's seq is between 70 and 80, and an even number (we don't want diags)
      sp_custom("CanPush", &current_sprite, 1);
     }
    }
    
    &save_x += 1;
    goto findpup;
   }

   //spawn the script that will perform checks and determine if Dink should engage a move-able sprite he is touching
   &save_y = &current_sprite;
   &save_x = spawn("pup");
   run_script_by_number(&save_x, "validate");
   &save_x = &return;
  
  validated:
   //check if any checks failed
   if (&save_x > 0)
   {
    //failed - end this
    sp_custom("reset-required", &current_sprite, 1);
    goto touchend;    
   }
   
   &save_x = sp_custom("HBOX", &current_sprite, -1);
   if (&save_x <= 0)
   {
    //hardbox values weren't provided.. try to auto detect if Dink is in position/direction to push a sprite without hardbox values.
    external("phispull", "auto_detect", &current_sprite);
    if (&return == 0)
    {
     sp_custom("reset-required", &current_sprite, 1);   
     goto touchend;
    }
    else
    {
     //Since there was a "wait()", let's assure dink's original sequence HAS NOT changed.
     //and also assure his position and direction haven't changed
     &save_x = sp_custom("HBOX", &current_sprite, -1);
     if (&save_x == 0) 
     {
      //seq check
      &save_x = sp_pseq(1, -1);
      &save_y = sp_custom("pseq-origin", &current_sprite, -1);
      if (&save_x != &save_y)
      {
       //it has changed - end it
       sp_custom("reset-required", &current_sprite, 1);
       kill_this_task();
      }
     
      //pos check
      &save_x = sp_x(1, -1);
      &save_y = sp_custom("dinkox", &current_sprite, -1);
      if (&save_x != &save_y)
      {
       //it has changed - end it
       sp_custom("reset-required", &current_sprite, 1);
       goto pupend;
      }  
      &save_x = sp_y(1, -1);
      &save_y = sp_custom("dinkoy", &current_sprite, -1);
      if (&save_y != &save_y)
      {
       //it has changed - end it
       sp_custom("reset-required", &current_sprite, 1);
       goto pupend;
      } 
     
      //dir check
      &save_x = sp_dir(1, -1);
      &save_y = sp_custom("pushdir", &current_sprite, -1);
      if (&save_x != &save_y)
      {
       //it has changed - end it
       sp_custom("reset-required", &current_sprite, 1);
       kill_this_task();  
      }  
     }
    }
   }

   /////////
   //TIMER//
   /////////
   //start the timer before push can activate
    
    //this will be the counter before push activates
    sp_custom("pushtimer", &current_sprite, 0);
    
   timerwaitloop:
    //skip the rest of this if no pushdelay is set.
    &save_x = sp_custom("pushdelay", &current_sprite, -1);
    if (&save_x > 0)
    {
     sp_custom("timerdone", &current_sprite, 0);
     &save_y = &current_sprite;
     &save_x = spawn("hup");
     sp_custom("timerscript", &current_sprite, &save_x);
    }
    else
    {
     goto timercont;
    }

    wait(0);
   
    //retrieve the old x, compare it against the new x
    &save_x = sp_custom("dinkox", &current_sprite, -1);
    &save_y = sp_x(1, -1);
    if (&save_x == &save_y)
    { 
     //retrieve the old y, compare it against the new y
     &save_x = sp_custom("dinkoy", &current_sprite, -1);
     &save_y = sp_y(1, -1);
     if (&save_x == &save_y)
     { 
      //retrieve the old direction, compare it agianst the new direction
      &save_x = sp_custom("pseq-origin", &current_sprite, -1);
      &save_y = sp_pseq(1, -1);
      if (&save_x == &save_y)
      {
       //everything is still correct for push/pull
       //check if the timer is done and if not, loop again
       &save_x = sp_custom("timerdone", &current_sprite, -1);
       if (&save_x > 0)
       {
        goto timercont;
       }
       else
       {
        goto timerwaitloop;
       }
      }
     }
    }
    
    //Dinks coords or sequence has changed, this tells us he's no longer pushing against the sprite
    &save_x = sp_custom("timerscript", &current_sprite, -1);
    if (&save_x > 0)
    {
     run_script_by_number(&save_x, "killtimer");
    }
    sp_custom("reset-required", &current_sprite, 1);    
    goto touchend; 
   /////////////
   //TIMER END//
   /////////////
 
 timercont:
    //initiate push and pull
    external("PhisHyb", "hybrid", &arg1);

touchend: 
 //since this touch proc is the start of the external chain, everything will lead back here
 //so I just made a custom key that can be changed to let this part know whether to do the sprite reset.
 &save_x = sp_custom("reset-required", &current_sprite, -1);
 if (&save_x == 1)
 {  
  //reset the "reset-required" custom key to 0
  sp_custom("reset-required", &current_sprite, 0);
  
  //kill off the shadow sprite used to retrieve and identify this moveable object
  &save_x = sp_custom("PP-Shadow", &current_sprite, -1);
  &save_y = sp_custom("PP-Parent", &save_x, -1);
  if (&save_y == &current_sprite)
   sp_active(&save_x, 0);
  
  //reset other custom keys
  sp_custom("CanPush", &current_sprite, 0);
  sp_custom("speedlock", &current_sprite, 0);
  sp_custom("SkipSpeedReset", &current_sprite, 0);

  &save_x = sp_custom("terminated", &current_sprite, -1);
  if (&save_x <= 0)
  {
   //only reset touch damage if a manual termination hasn't been called
   sp_touch_damage(&current_sprite, -1);  
  } 
 }
 
 kill_this_task(); 
}

void terminate(void)
{ 
 //Push/Pull is being manually terminated out of course.
 //&arg1 = sprite being terminated (is passed), otherwise, it's &current_sprite.
 sp_custom("terminated", &current_sprite, 1);
 if (&arg1 > 0)
 {
  sp_touch_damage(&arg1, 0);
  external("PhisEnd", "end", 9, 0, 0, &arg1);
 }
 else
 {
  sp_touch_damage(&current_sprite, 0);
  external("PhisEnd", "end", 9, 0, 0, &current_sprite);
 }

 kill_this_task();
}

void initiate(void)
{
 //there used to be more involved - but now most of it is handled when the sprite is terminated.
 if (&arg1 > 0)
 {
  sp_touch_damage(&arg1, -1);
  sp_custom("terminated", &arg1, 0);
 }
 else
 {
  sp_touch_damage(&current_sprite, -1);
  sp_custom("terminated", &current_sprite, 0);
 }
 kill_this_task();
}

void MoveActive(void)
{
 //checks if Dink is currently moving a sprite 
 //if yes, sprite number will be returned
 if (&arg1 > 0)
 {
  goto movespriteactive;
 }

 //No specific sprite passed to proc - check if Dink is moving a sprite on this screen
 &save_x = 0;
moveactive:
 //get invisible shadow sprites, there is one for each moveable object that is currently being touched
 &save_x = get_next_sprite_with_this_brain(15, 0, &save_x);
 if (&save_x > 0)
 {
  &save_y = sp_custom("PP-Parent", &save_x, -1);
  &save_y = sp_custom("PP-Shadow", &save_y, -1);
  if (&save_y == &save_x)
  {
   &save_y = sp_custom("PP-Parent", &save_x, -1);
   &save_y = sp_custom("hybactive", &save_y, -1);
   if (&save_y != 1)
   {
    goto moveactiveinc;
   }
   
   //this will run if any of the above return 1
   &save_y = sp_custom("PP-Parent", &save_x, -1);
   goto moveactiveend;
  }
  
 moveactiveinc:
  &save_x += 1;
  goto moveactive;
 }
 else
 {
  return(0); 
  kill_this_task();
 } 

//checking if Dink is pushing a particular sprite, stored in &arg1
movespriteactive:
 &save_x = sp_custom("hybactive", &arg1, -1);
 if (&save_x != 1)
 {
  return(0);   
  kill_this_task();
 }

moveactiveend:
 //return the sprite number of the sprite being moved.
 if (&arg1 > 0)
 {
  &save_y = &arg1;
 }
 return(&save_y);   
 kill_this_task();  
}

void talk(void)
{
//NO LONGER REQUIRED - Left in to maintain older push and pull version compatibility.
//So authors don't have to re-edit all their moveable sprite scripts.
 //determines if Dink is pushing the sprite, or isn't.
 //To be able to customise talk lines.
sp_custom("talkreturn", &current_sprite, 0); 
talkloop:
 &save_x = sp_custom("hybactive", &current_sprite, -1);
 if (&save_x != 1)
 { 
  sp_custom("talkreturn", &current_sprite, 1);
  kill_this_task();
 }

 sp_custom("talkreturn", &current_sprite, 2);
 kill_this_task();
}

void stopmove(void)
{
 //Stop an object from moving any further. Will be as if it's hit hardness (but it hasn't)
 //This proc isn't really required, since the custom key can be set directly.
 //But I think I used it in some push and pull puzzles in the demo dmod so I'm leaving it here.
 sp_custom("stopmove", &current_sprite, 1);
 kill_this_task();
}

void startmove(void)
{
 //undo the stopmove custom key (reset it to 0)
 //This proc isn't really required, since the custom key can be set directly.
 //But I think I used it in some push and pull puzzles in the demo dmod so I'm leaving it here.
 sp_custom("stopmove", &current_sprite, 0);
 kill_this_task();
}

void kill(void)
{
 kill_this_task();
}