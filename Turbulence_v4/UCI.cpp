

#include "MoveGeneration.h"
#include "Evaluation.h"
#include "Board.h"
#include "Search.h"
#include "const.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

const std::string start_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

std::vector<std::string> position_commands = { "position", "startpos", "fen", "moves" };
std::vector<std::string> go_commands = { "go", "movetime", "wtime", "btime", "winc", "binc", "movestogo" };
Board main_board;

int perft_depth;
std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    const auto start = str.find_first_not_of(whitespace);
    const auto end = str.find_last_not_of(whitespace);

    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }

    return str.substr(start, end - start + 1);
}

std::string TryGetLabelledValue(const std::string& text, const std::string& label, const std::vector<std::string>& allLabels, const std::string& defaultValue = "") {
    // Trim leading and trailing whitespace
    std::string trimmedText = trim(text);

    // Find the position of the label in the trimmed text
    size_t labelPos = trimmedText.find(label);
    if (labelPos != std::string::npos) {
        // Determine the start position of the value
        size_t valueStart = labelPos + label.length();
        size_t valueEnd = trimmedText.length();

        // Iterate through allLabels to find the next label position
        for (const std::string& otherID : allLabels) {
            if (otherID != label) {
                size_t otherIDPos = trimmedText.find(otherID, valueStart);
                if (otherIDPos != std::string::npos && otherIDPos < valueEnd) {
                    valueEnd = otherIDPos;
                }
            }
        }

        // Extract the value and trim leading/trailing whitespace
        std::string value = trimmedText.substr(valueStart, valueEnd - valueStart);
        return trim(value);
    }

    return defaultValue;
}
int TryGetLabelledValueInt(const std::string& text, const std::string& label, const std::vector<std::string>& allLabels, int defaultValue = 0)
{
    // Helper function TryGetLabelledValue should be implemented as shown earlier
    std::string valueString = TryGetLabelledValue(text, label, allLabels, std::to_string(defaultValue));

    // Extract the first part of the valueString up to the first space
    std::istringstream iss(valueString);
    std::string firstPart;
    iss >> firstPart;

    // Try converting the extracted string to an integer
    try
    {
        return std::stoi(firstPart);
    }
    catch (const std::invalid_argument& e)
    {
        // If conversion fails, return the default value
        return defaultValue;
    }
}
std::vector<std::string> splitStringBySpace(const std::string& str)
{
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;

    while ((end = str.find(' ', start)) != std::string::npos)
    {
        if (end != start)
        { // Ignore multiple consecutive spaces
            tokens.push_back(str.substr(start, end - start));
        }
        start = end + 1;
    }

    // Push the last token if it's not empty
    if (start < str.size())
    {
        tokens.push_back(str.substr(start));
    }

    return tokens;
}

uint64_t Perft(Board& board, int depth)
{

    if (depth == 0)
    {
        return 1ULL;
    }

    std::vector<Move> move_list;
    //int i;

    uint64_t nodes = 0;

    Generate_Legal_Moves(move_list, board, false);
    //if (depth == 1)
    //{
    //    return move_list.size();
    //}
    //std::cout << (move_list.size()) << ("\n");
    //int movecount = move_list.size();
    for (Move& move : move_list)
    {
        int lastEp = board.enpassent;
        uint64_t lastCastle = board.castle;
        int lastside = board.side;
        int captured_piece = board.mailbox[move.To];
        //ulong lastZobrist = Zobrist;
        MakeMove(board, move);
        //u64 nodes_added
        if (isMoveValid(move, board))//isMoveValid(move, board)
        {
            //std::cout<<(Perft(board, depth - 1));
            uint64_t nodes_added = Perft(board, depth - 1);
            nodes += nodes_added;

            if (depth == perft_depth)
            {
                printMove(move);
                std::cout << (":") << nodes_added << "\n";

            }


        }
        UnmakeMove(board, move, captured_piece);

        board.enpassent = lastEp;
        board.castle = lastCastle;
        board.side = lastside;
        //Zobrist = lastZobrist;

    }
    //std::cout << (nodes);
    return nodes;
}

int CalculateTime(int time, int incre)
{
    return time / 20 + incre / 2;
}

void ProcessUCI(std::string input)
{
    //std::cout << (input) << "\n";
    //std::string input = "This  is   a  sample string";
    std::vector<std::string> Commands = splitStringBySpace(input);
    std::string main_command = Commands[0];

    if (main_command == "uci")
    {
        std::cout << "uciok" << "\n";
        std::cout << "id name Turbulence" << "\n";;
        std::cout << "id author ksw0518" << "\n";;
    }
    else if (main_command == "isready")
    {
        std::cout << "readyok"<< "\n";

    }
    else if (main_command == "quit")
    {
        exit(0);
    }
    else if (main_command == "position")
    {
        if (Commands[1] == "startpos")
        {
            if (Commands.size() == 2)
            {
                parse_fen(start_pos, main_board);
            }
            else
            {
                //std::cout << ("fuck");
                parse_fen(start_pos, main_board);
                std::string moves_in_string = TryGetLabelledValue(input, "moves", position_commands);
                //std::cout << (moves_in_string);
                if (moves_in_string != "") // move is not empty
                {
                    std::vector<std::string> moves_seperated = splitStringBySpace(moves_in_string);
                    std::vector<Move> moveList;
                    //std::cout << (moves_seperated[0]);

                    for (int i = 0; i < moves_seperated.size(); i++)
                    {
                        std::string From = std::string(1, moves_seperated[i][0]) + std::string(1, moves_seperated[i][1]);
                        std::string To = std::string(1, moves_seperated[i][2]) + std::string(1, moves_seperated[i][3]);
                        std::string promo = "";
                        
                        //std::cout << ("\n");
                        //std::cout << (From);
                        //std::cout << ("\n");
                        //std::cout << (To);
                        //if (moves_seperated[i] == "h7h8q")
                        //{
                        //    std::cout << std::to_string(moves_seperated[i][4]);
                        //}
                        if (moves_seperated[i].size() > 4)
                        {
                            promo = std::string(1, (moves_seperated[i][4]));
                            //std::cout << promo;
                        }
                        Move move_to_play;
                        move_to_play.From = GetSquare(From);
                        move_to_play.To = GetSquare(To);

                        //std::cout << CoordinatesToChessNotation(move_to_play.From)<<"\n";
                       // std::cout << CoordinatesToChessNotation(move_to_play.To) << "\n";

                        moveList.clear();
                        Generate_Legal_Moves(moveList, main_board, false);

                        for (int j = 0; j < moveList.size(); j++)
                        {
                            //Console.WriteLine("12");
                            //nodes = 0;

                            if ((move_to_play.From == moveList[j].From) && (move_to_play.To == moveList[j].To)) //found same move
                            {



                                if ((moveList[j].Type & knight_promo) != 0) // promo
                                {
                                    //std::cout << "promo" << "\n";
                                    //std::cout << promo << "\n";
                                    if (promo == "q")
                                    {
                                        //std::cout << "qpromo";
                                        if ((moveList[j].Type == queen_promo) || (moveList[j].Type == queen_promo_capture))
                                        {
                                            MakeMove(main_board, moveList[j]);
                                            break;

                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "r")
                                    {
                                        if ((moveList[j].Type == rook_promo) || (moveList[j].Type == rook_promo_capture))
                                        {
                                            MakeMove(main_board, moveList[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "b")
                                    {
                                        if ((moveList[j].Type == bishop_promo) || (moveList[j].Type == bishop_promo_capture))
                                        {
                                            MakeMove(main_board, moveList[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "n")
                                    {
                                        if ((moveList[j].Type == knight_promo) || (moveList[j].Type == knight_promo_capture))
                                        {
                                            MakeMove(main_board, moveList[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                }
                                else
                                {
                                    //Console.WriteLine(MoveType[moveList[j].Type]);
                                    //Console.WriteLine(ascii_pieces[moveList[j].Piece]);
                                   // printMove(moveList[j]);
                                    //Console.Write(" ");
                                    //Move_to_do.Add(moveList[j]); 
                                    MakeMove(main_board, moveList[j]);
                                    //if (isMoveIrreversible(moveList[j]))
                                    //{
                                    //    //Console.WriteLine("aaa");
                                    //    Repetition_table.Clear();
                                    //    main_board.halfmove = 0;
                                    //}
                                    //Repetition_table.Add(main_Zobrist);
                                    main_board.halfmove++;


                                    break;
                                }



                            }
                        }
                    
                    }

                }
            }
           
        }
        else if (Commands[1] == "fen")
        {
            std::string fen = TryGetLabelledValue(input, "fen", position_commands);
            std::string moves = TryGetLabelledValue(input, "moves", position_commands);

            if (moves == "")
            {
                parse_fen(fen, main_board);
            }
            else
            {
                parse_fen(fen, main_board);
                std::string moves_in_string = TryGetLabelledValue(input, "moves", position_commands);
                if (moves_in_string != "") // move is not empty
                {
                    std::vector<std::string> moves_seperated = splitStringBySpace(moves_in_string);
                    std::vector<Move> moveList;


                    for (int i = 0; i < moves_seperated.size(); i++)
                    {
                        std::string From = std::string(1, moves_seperated[i][0]) + std::string(1, moves_seperated[i][1]);
                        std::string To = std::string(1, moves_seperated[i][2]) + std::string(1, moves_seperated[i][3]);
                        std::string promo = "";
                        if (moves_seperated[i].size() > 4)
                        {
                            promo = std::string(1, (moves_seperated[i][4]));
                        }
                        Move move_to_play;
                        move_to_play.From = GetSquare(From);
                        move_to_play.To = GetSquare(To);

                        //std::cout << CoordinatesToChessNotation(move_to_play.From);
                        //std::cout << CoordinatesToChessNotation(move_to_play.To);
                        //std::cout << promo;
                        moveList.clear();
                        Generate_Legal_Moves(moveList, main_board, false);

                        for (int j = 0; j < moveList.size(); j++)
                        {
                            //Console.WriteLine("12");
                            //nodes = 0;

                            if ((move_to_play.From == moveList[j].From) && (move_to_play.To == moveList[j].To)) //found same move
                            {



                                if ((moveList[j].Type & knight_promo) != 0) // promo
                                {
                                    if (promo == "q")
                                    {
                                        if ((moveList[j].Type == queen_promo) || (moveList[j].Type == queen_promo_capture))
                                        {
                                            MakeMove(main_board, moveList[j]);
                                            break;

                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "r")
                                    {
                                        if ((moveList[j].Type == rook_promo) || (moveList[j].Type == rook_promo_capture))
                                        {
                                            MakeMove(main_board, moveList[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "b")
                                    {
                                        if ((moveList[j].Type == bishop_promo) || (moveList[j].Type == bishop_promo_capture))
                                        {
                                            MakeMove(main_board, moveList[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                    else if (promo == "n")
                                    {
                                        if ((moveList[j].Type == knight_promo) || (moveList[j].Type == knight_promo_capture))
                                        {
                                            MakeMove(main_board, moveList[j]);
                                            break;
                                            //Move_to_do.Add(moveList[j]);
                                        }
                                    }
                                }
                                else
                                {
                                    //Console.WriteLine(MoveType[moveList[j].Type]);
                                    //Console.WriteLine(ascii_pieces[moveList[j].Piece]);
                                   // printMove(moveList[j]);
                                    //Console.Write(" ");
                                    //Move_to_do.Add(moveList[j]); 
                                    MakeMove(main_board, moveList[j]);
                                    //if (isMoveIrreversible(moveList[j]))
                                    //{
                                    //    //Console.WriteLine("aaa");
                                    //    Repetition_table.Clear();
                                    //    main_board.halfmove = 0;
                                    //}
                                    //Repetition_table.Add(main_Zobrist);
                                    main_board.halfmove++;


                                    break;
                                }



                            }
                        }

                    }

                }
            }

           
            //std::cout << TryGetLabelledValue(input, "fen", position_commands);
        }


    }
    else if (main_command == "go")
    {
        if (Commands[1] == "perft")
        {

            perft_depth = std::stoi(Commands[2]);

            //std::cout << (perft_depth);
            auto start = std::chrono::high_resolution_clock::now();
            uint64_t nodes = Perft(main_board, perft_depth);
            auto end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double, std::milli> elapsedMS = end - start;
            //std::chrono::duration<double, std::milli> elapsedS = end - start;

            float second = elapsedMS.count() / 1000;

            double nps = nodes / second;
            double nps_in_millions = nps / 1000000.0;

            std::cout << "Nodes: ";
            std::cout << (nodes);
            std::cout << " Time: ";
            std::cout << elapsedMS.count();
            std::cout << "ms NPS : ";
            std::cout << std::fixed << std::setprecision(1) << nps_in_millions << "M n/s" << std::endl;

        }
        else if (Commands[1] == "depth")
        {
            //Negamax_nodecount = 0;
            int depth = std::stoi(Commands[2]);
            IterativeDeepening(main_board, depth);

        }
        else if (Commands[1] == "movetime")
        {
            int movetime = std::stoi(Commands[2]);
            IterativeDeepening(main_board, 64, movetime);
        }
        else if (Commands[1] == "wtime")
        {
            int wtime = TryGetLabelledValueInt(input, "wtime", go_commands);
            int btime = TryGetLabelledValueInt(input, "btime", go_commands);
            int winc = TryGetLabelledValueInt(input, "winc", go_commands);
            int binc = TryGetLabelledValueInt(input, "binc", go_commands);
           
            int searchtime;

            if (main_board.side == White)
            {
                searchtime = CalculateTime(wtime, winc);
            }
            else
            {
                searchtime = CalculateTime(btime, binc);
            }
            IterativeDeepening(main_board, 64, searchtime);
        }
        //else if (Commands[1] == "perft")
        //{

        //}
    }
    else if (main_command == "show")
    {
        PrintBoards(main_board);
        print_mailbox(main_board.mailbox);
    }
    else if (main_command == "eval")
    {
        std::cout << ("evaluation: ") << Evaluate(main_board) << "cp ";
        if (main_board.side == White)
        {
            std::cout << ("(White's perspective)\n");
        }
        else
        {
            std::cout << ("(Black's perspective)\n");


            std::cout << ("White's perspective: ") << -Evaluate(main_board) << "cp \n";
        }



    }

}


static void InitAll()
{
    InitializeBetweenTable();
    InitializeLeaper();
    init_sliders_attacks(1);
    init_sliders_attacks(0);
    init_tables();

}
int main()
{
    InitAll();
    
    parse_fen(start_pos, main_board);
    std::vector<Move> move_list;
    Generate_Legal_Moves(move_list, main_board, false);
    //PrintLegalMoves(move_list);

    //print_tables();
    while (true)
    {
        std::string input;

        std::getline(std::cin, input);
        if (input != "")
        {
            ProcessUCI(input);
        }

    }
}