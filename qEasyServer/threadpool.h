#pragma once

#include<pthread.h>
#include<list>
#include"locker.h"

template <typename T>
class threadpool
{
public:
	threadpool(int thread_number = 8, int max_request = 10000);
	~threadpool();
	bool append(T* request, int state);
private:
	static void* worker(void* arg);
	void run();

private:
	 int m_thread_number;
	 int m_max_request;
	 pthread_t* m_threads;
	 std::list<T*> m_workqueue;
	 locker m_queuelocker;
	 sem m_queuestat;
};

template <typename T>
threadpool<T>::threadpool(int thread_number , int max_request) :m_thread_number(thread_number),m_max_request(max_request)
{
	if (thread_number<=0 || max_request<=0)
	{
		throw std::exception();
	}
	m_threads = new pthread_t[m_thread_number];
	if (!m_threads)
	{
		throw std::exception();
	}
	for (int i = 0; i < thread_number; i++)
	{
		if (pthread_create(&m_threads[i],NULL,worker,this)!=0)
		{
			delete[] m_threads;
			throw std::exception();
		}
		if (pthread_detach(m_threads[i]))
		{
			delete[] m_threads;
			throw std::exception();
		}
	}

}
template <typename T>
threadpool<T>::~threadpool()
{
	delete[] m_threads;
}

template<typename T>
bool threadpool<T>::append(T* request, int state)
{
	m_queuelocker.lock();
	if (m_workqueue.size() >= m_max_request)
	{
		m_queuelocker.unlock();
		return false;
	}
	request->m_state = state;
	m_workqueue.push_back(request);
	m_queuelocker.unlock();
	m_queuestat.post();
	return true;
}

template<typename T>
void* threadpool<T>::worker(void* arg)
{
	threadpool* pool = (threadpool*)arg;
	pool->run();
	return pool;
}

template<typename T>
void threadpool<T>::run()
{
	while (true)
	{
		m_queuestat.wait();
		m_queuelocker.lock();
		if (m_workqueue.empty())
		{
			m_queuelocker.unlock();
			continue;
		}
		T* request = m_workqueue.front();
		m_workqueue.pop_front();
		m_queuelocker.unlock();
		if (!request)
		{
			continue;
		}
		//这里就开始处理http了
		if (request->m_state == 0)
		{
			if (request->read_once())
			{
				request->improv = 1;
				request->process();
			}
			else {
				request->improv = 1;
				request->timer_flag = 1;
			}
		}
		else {
			if (request->write())
			{
				request->improv = 1;
			}
			else { //不是keep-alive 即立刻设置超时，断开连接
				request->improv = 1;
				request->timer_flag = 1;
			}
		}
		
	}
}
