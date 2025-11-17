#pragma once

#include "IETThread.h"
#include "WorkerThread.h"
#include "IWorkerAction.h"

#include <queue>
#include <unordered_map>

using namespace std;

class ThreadPool : public IETThread, public IFinishedTask {
public:
	ThreadPool(int _workerCount);
	~ThreadPool();

	void StartScheduling();
	void StopScheduling();
	void WaitAll();

	void ScheduleTask(IWorkerAction* _task);

	bool isRunning = false;

private:
	void run() override;
	void OnFinishedTask(int id) override;

	int workerCount = 1;

	queue<IWorkerAction*> PendingTasks;
	unordered_map<int, WorkerThread*> ActiveThreads;
	queue<WorkerThread*> InactiveThreads;
};