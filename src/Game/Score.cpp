
#pragma once

/*
    Sorted list of the best scores.
*/

static constexpr u32 s_max_highscore_count = 10;
static u32 s_highscores[s_max_highscore_count] = {0};


struct Score_Package
{
	u32 score = 0;
	u32 ranking = 0;
};
static Score_Package s_last_earned_score = {0};


static u32 add_new_score(u32 score)
{
	u32 ranking = 0;
	
	for(u32 i = 0; i < s_max_highscore_count; ++i)
	{
		if(score > s_highscores[i])
		{
			// Shift down the other scores, exept the last one.
			for(u32 y = s_max_highscore_count - 1; y > i; --y)
				s_highscores[y] = s_highscores[y - 1];
			
			s_highscores[i] = score;
			ranking = i + 1;
			break;
		}
	}
	
	s_last_earned_score = { score, ranking };
	
	return ranking;
}
