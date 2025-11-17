#include "WorkerThread.h"

WorkerThread::WorkerThread(int _id, IFinishedTask* _onDone) {
	id = _id;
	onDone = _onDone;
}

WorkerThread::~WorkerThread() {

}

void WorkerThread::AssignTask(IWorkerAction* _task) {
	this->task = _task;
}

void WorkerThread::run() {
	if (task != nullptr) {
		task->OnStartTask();
	}

	if (onDone != nullptr) {
		onDone->OnFinishedTask(id);
	}
}