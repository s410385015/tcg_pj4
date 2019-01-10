#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include <deque>
#include "board.h"
#include "action.h"
#include "TD.h"
#include "weight.h"
class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};

/**
 * random environment
 * add a new random tile to an empty cell
 * 2-tile: 90%
 * 4-tile: 10%
 */
class rndenv : public random_agent {
public:
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
		space({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }), 
		space_each_op({12,13,14,15,0,4,8,12,0,1,2,3,3,7,11,15})
		{}


	virtual action take_action(const board& after) {
		

		total++;
		std::shuffle(space.begin(), space.end(), engine);
		if(bags.size() <2) FillIn();
		
	
		board::cell tile=GenerateTile(after);
		int last_move=after.getLastMove();
		//std::cout<<last_move<<std::endl;

		if(last_move==-1){
			for (int pos : space) {
				if (after(pos) != 0) continue;	
						
				return action::place(pos, tile,bags.front());
			}
		}else
		{
			std::shuffle(space_each_op[last_move].begin(), space_each_op[last_move].end(), engine);
			for(int pos : space_each_op[last_move])
			{
				//std::cout<<pos<<" ";
				if (after(pos) != 0) continue;			
				return action::place(pos, tile,bags.front());
			}
		}
		return action();
	}



	board::cell GenerateTile(const board& b)
	{
		board::cell max_tile=b.GetMaxTile();
		
		board::cell tile=-1;
		if(max_tile>=7){

		
			if((rand()%21==0)&&((bonus+1)/total)<(1.0/21.0))
			{
				tile=4+rand()%(max_tile-7+1);
				bonus++;
			}
			else
			{
				tile= bags.front();
				bags.pop_front();
				
			}
			
		}
		else
		{
		 	tile= bags.front();
			bags.pop_front();
			//std::cout<<"?"<<std::endl;
		}
		//std::cout<<max_tile<<std::endl;
		//std::cout<<tile<<std::endl;
		return tile;
	}



	void FillIn()
	{	
		std::array<int,12> tmp{1,1,1,1,2,2,2,2,3,3,3,3};
		std::shuffle(tmp.begin(),tmp.end(),engine);
		for(int i=0;i<12;i++)
			bags.push_back(tmp[i]);
			
	}
	void ResetBags()
	{
		bags.clear();
		FillIn();
		total=bonus=0;
	}
private:
	std::array<int, 16> space;
	std::array<std::array<int,4>,4> space_each_op;
	std::deque<int> bags;
	float total;
	float bonus;
};

/**
 * dummy player
 * select a legal action randomly
 */
class player : public random_agent {
public:
	player(const std::string& args = "") : random_agent("name=dummy role=player " + args),
		opcode({  0,1, 2, 3 }),
		space_each_op({12,13,14,15,0,4,8,12,0,1,2,3,3,7,11,15}) {
			path.reserve(20000);
			td.add(new weight(1<<6*sizeof(float),{0, 1, 2, 3, 4, 5}));
			td.add(new weight(1<<6*sizeof(float),{4, 5, 6, 7, 8, 9}));
			td.add(new weight(1<<6*sizeof(float),{0, 1, 2, 4, 5, 6 }));
			td.add(new weight(1<<6*sizeof(float),{4, 5, 6, 8, 9, 10 }));
		}

	virtual action take_action(const board& before) {

		return SelectMove(before);
		//std::shuffle(opcode.begin(), opcode.end(), engine);
		/*
		board:: reward _max=-1;
		action act=action();
		for (int op : opcode) {
			board::reward reward = board(before).slide(op);
			
			if (reward == -1)
				continue;
			
			for(int pos : space_each_op[op])
			{
				board after_slide(before);
				after_slide.slide(op);
				if(after_slide(pos)!=0) 
					continue;

				
				board after(after_slide);

				after.place(pos,before.getNextTile(),1);

				for (int op1 : opcode) {
					
					
					board::reward extra = board(after).slide(op1);
					if (extra == -1)
						continue;
					reward+=extra;
					//std::cout<<op<<" "<<op1<<" "<<reward<<" "<<reward1<<std::endl;
					
				}
				
			}
			
			if (reward >_max){ 
						//return action::slide(op);
				_max=reward;
				act=action::slide(op);
			}
			
		}
		

		*/
		//return act;
	}

	action SelectMove(const board& before)
	{
		
		board:: reward _max=INT_MIN;
		action act=action();
		
		state move;
		
		move.SetBefore(before);
		move.SetHint(before.getNextTile());
		for (int op : opcode) {
			board after(before);
			board::reward reward = after.slide(op);
			//std::cout<<op<<" "<<reward<<std::endl;
			if (reward == -1)
				continue;

			board:: reward new_reward=reward+td.estimate(after,before.getNextTile()-1);
			
			if(new_reward>_max)
			{
				_max=new_reward;
				act=action::slide(op);
				move.SetAfter(after);
				move.SetReward(reward);
				move.SetValue(new_reward);
				
			}
		}

		path.emplace_back(move);
		return act;
	}

	void TD_update()
	{
		td.UpdateEpisode(path);
		
	}
	
	void TD_save()
	{
		td.Save();
	}
	void TD_load()
	{
		td.Load();
	}
private:
	std::array<int, 4> opcode;
	std::array<std::array<int,4>,4> space_each_op;
	std::vector<state> path;
	TD td;
};
