# Puzzle-a-day Solver

## Group Members
- Yannis BELKHITER
- Benoît FAURE
- Romain STORAÏ
- Lucas TEXIER

## Project Overview
This repository contains the source code and documentation for solving the "Puzzle-a-day" problem. The problem is inspired by a famous puzzle and involves placing 10 different pieces on a calendar grid. The goal is to select a day of the week, a day of the month, and a month, and then arrange the 10 blue pieces on the grid without overlapping. The challenge is to leave three selected cells uncovered, corresponding to the date of the day in the year.

## Table of Contents
1. [Introduction](#introduction)
2. [Problem Statement](#problem-statement)
3. [Our Approach](#our-approach)
4. [Results and Analysis](#results-and-analysis)
5. [Conclusion](#conclusion)
6. [Usage](#usage)
7. [Contributing](#contributing)
8. [License](#license)

## Introduction
The project tackles the "Puzzle-a-day" problem proposed by Professor Gérard Dray. The objective is to explore the complexity and recursion aspects of the problem, aiming to find the number of solutions for each day of the year and analyze the algorithm's performance.

## Problem Statement
The main question we address is whether it's possible to complete the puzzle every day of the year. The project involves developing an algorithm to find the number of possible solutions per day and studying its complexity.

## Our Approach
We implemented a recursive algorithm in C, with two main modelings. The first model represents days, dates, and months using integers, while the second optimizes the code complexity by considering the grid as 8 rows of 8 bits each.

## Results and Analysis
The algorithm successfully found solutions for the majority of combinations. The results are documented in the 'solutions_per_day' spreadsheet, showcasing the number of solutions and execution times. We also analyzed the distribution of solutions and identified combinations without solutions.

## Conclusion
In conclusion, our project provides insights into the solvability of the "Puzzle-a-day" problem for different combinations. While some combinations lack solutions, removing certain constraints allows finding solutions for all combinations.
