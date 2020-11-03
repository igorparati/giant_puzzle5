#include <iostream>
#include <algorithm>
#include <random> 
#include <vector> 
#include <chrono>

using namespace std;

//who_won look at the deck and verify who won
//it returns 1 if player won (not by bj)
//it returns 2 if player won by bj
//it returns 3 if player lost
//it returns 0 if it's a draw

int real_i(int n_i){
    if(n_i == 2 || n_i == 6)
        return -1;
    if(n_i == 4 || n_i == 8)
        return 1;
    return 0;
}

void sum(vector<int>::iterator deck, int soft, int* sum, int* bj, int start, int* finish){
    int n_i = 0; //number of i's
    int sum_i = 0; //sum_i = Re(i^n_i)
    int part_sum = 0; // partial sum
    bool bust = false;
    bool converted = false; //true if As was already set as 1
    bool there_is_As = false; //true if As already appeared
    *bj = 0; //we don't know if it's a bj yet

    //initial sum
    //black jack cases
    if((deck[start] == 1 && deck[start+1] == 10) || (deck[start] == 10 && deck[start+1] == 1)){
        *bj = 1;
        part_sum = 21;
    }
    //other initial As cases
    else if(deck[start] == 1 && deck[start+1] == 1){
        there_is_As = true;
        converted = true; //As is set to 1
        part_sum = 12;
    }
    else if(deck[start] == 1){
        there_is_As = true;
        part_sum = 11 + deck[start+1];
    }
    else if(deck[start+1] == 1){
        there_is_As = true;
        part_sum = 11 + deck[start];
    }
    else
        part_sum = deck[start] + deck[start+1];

    //check if there are i cards
    if(deck[start] == 0)
        n_i++;
    if(deck[start+1] == 0)
        n_i++;

    //update initial sum with i cards
    sum_i = real_i(n_i);

    //play the game
    int i = start+2;
    int next_card;
    while(bust == false && part_sum + sum_i < soft){
        next_card = deck[i];

        //i cards
        if(next_card == 0)
            n_i++;

        //As cards
        if(next_card == 1 && converted == false){ //when As is 11
            part_sum += 11;
            there_is_As = true;
        }
        else if(next_card == 1 && converted == true){ //when As is 1
            part_sum += 1;
        }
        else
            part_sum += next_card;
        
        sum_i = real_i(n_i);

        //if bust and As is 11, then change As to 1
        if(part_sum + sum_i > 21 && there_is_As == true && converted == false){
            part_sum += -10; // -11 + 1
            converted = true;
        }

        if(part_sum + sum_i > 21)
            bust = true;
        i++;
    }

    *sum = part_sum + sum_i;
    *finish = i;
}

int who_won(vector<int>::iterator deck, int soft){
    //p indicates player, h indicates house
    //_bj = 0 is false and _bj = 1 is true
    int p_sum, p_bj, h_sum, h_bj;

    //where the player (or house) sequence starts
    int start;

    sum(deck, soft, &p_sum, &p_bj, 2, &start); //the first two cards are from house
    
    // cout << "player sum: " << p_sum << endl;
    // cout << "player blackjack: " << p_bj << endl;
    
    if(p_sum > 21)
        return 3;

    //the first two cards are from house, so we need to update the deck order
    deck[start-2] = deck[0];
    deck[start-1] = deck[1];

    sum(deck, 17, &h_sum, &h_bj, start-2, &start);

    // cout << "dealer sum:    " << h_sum << endl;
    // cout << "dealer blackjack: " << h_bj << endl;

    if((h_sum > 21 || p_sum > h_sum) && p_bj == 0)
        return 1;
    if((h_sum > 21 || p_sum > h_sum) && p_bj == 1)
        return 2;
    if(p_sum == h_sum && p_bj == 1 && h_bj == 0)
        return 2;
    if(p_sum == h_sum && p_bj == 0 && h_bj == 1)
        return 3;
    if(p_sum < h_sum)
        return 3;
    
    //otherwise, it's a draw
    return 0;
}

int main(){
    //create the deck
    //i is represented by 0
    //J,Q,K are represented by 10
    vector<int> deck {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,8,9,9,9,9,
                        10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};

    //total number of matchs and soft number
    int N = 100000000, soft;
    
    //counter variables
    //when player wins by blackjack n_player_won is not updated
    int n_player_won, n_player_lost, n_player_won_bj;

    //auxiliary counter
    int aux; 

    //edge
    float edge;

    for(int k = -1; k <= 21; k++){
        soft = k;
        n_player_won = 0;
        n_player_won_bj = 0;
        n_player_lost = 0;
        //run all episodes
        for(int i = 0; i < N; i++){
            //suffle the deck
            vector<int> deck_aux = deck;
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            shuffle(deck_aux.begin(), deck_aux.end(), std::default_random_engine(seed));

            // for(int j = 0; j < 60; j++){
            //     cout << deck_aux[j] << " ";
            // }
            // cout << endl;

            //who won
            aux = who_won(deck_aux.begin(), soft); 
            if (aux == 1){
                n_player_won++;
            }
            else if (aux == 2){
                n_player_won_bj++;
            }
            else if (aux == 3){
                n_player_lost++;
            }
        }

        //edge evaluation: edge = 1*p_player_lost - 1*p_player_win - 1.5*p_player_win_bj
        //to maximize precision, use this formula
        edge = (float)(2*n_player_lost - 2*n_player_won - 3*n_player_won_bj)/(2*N);
        cout << "edge com soft = " << soft << ": " << edge << endl;
    }

}