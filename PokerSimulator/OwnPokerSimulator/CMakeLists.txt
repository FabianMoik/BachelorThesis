cmake_minimum_required(VERSION 3.8)
project(OwnPokerSimulator)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        ai/NN_agent/NeuralNet.cpp
        ai/NN_agent/NeuralNet.h
        ai/NN_agent/Neuron.cpp
        ai/NN_agent/Neuron.h
        ai/ai.cpp
        ai/ai.h
        ai/aiFolder.cpp
        ai/aiFolder.h
        ai/aiCaller.cpp
        ai/aiCaller.h
        ai/aiOwn.cpp
        ai/aiOwn.h
        ai/aiRandom.cpp
        ai/aiRandom.h
        ai/aiRaiser.cpp
        ai/aiRaiser.h
        ai/aiSmart.cpp
        ai/aiSmart.h
        game_assets/action.cpp
        game_assets/action.h
        game_assets/card.cpp
        game_assets/card.h
        game_assets/cardDealer.cpp
        game_assets/cardDealer.h
        game_assets/deck.cpp
        game_assets/deck.h
        game_assets/game.cpp
        game_assets/game.h
        game_assets/information.cpp
        game_assets/information.h
        game_assets/player.cpp
        game_assets/player.h
        game_assets/table.cpp
        game_assets/table.h
        hand_eval/pokereval.cpp
        hand_eval/pokereval.h
        hand_eval/pokereval2.cpp
        hand_eval/pokereval2.h
        #test_cases/handEvaluatorTest.cpp
        #test_cases/simulatorTestCases.cpp
        #test_cases/testMultiThreading.cpp
        #test_cases/createPreflopTableFile.cpp
        #test_cases/testGettingWinChancePreflop.cpp
        #test_cases/testDifferentAIsForCorrectness.cpp
        test_cases/testSingleTableAllAgents.cpp
        #test_cases/testAgentsAgainstAgents.cpp
        #test_cases/normal_distribution_test.cpp
        #main.cpp
        hand_eval/2+2/2P2Evaluator.cpp
        hand_eval/2+2/2P2Evaluator.h
        hand_eval/pokermath.h
        hand_eval/pokermath.cpp
        random.cpp
        random.h
        rules.cpp
        rules.h
        statistics.cpp
        statistics.h
        util.cpp
        util.h hand_eval/handEvalCalc.cpp hand_eval/handEvalCalc.h )

add_executable(OwnPokerSimulator ${SOURCE_FILES})