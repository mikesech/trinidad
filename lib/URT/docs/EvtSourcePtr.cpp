EvtSourcePtr<File> ptr = loop.add(new File);
//Since the loop has not had a chance to delete the new \c File object,
//we can lock \c ptr, obtain a \c shared_ptr, and use it just like a regular
//pointer within the if statement.
if(boost::shared_ptr<File> p = ptr.lock())
{
	p->func();
}

eventLoop.run();

//If the event loop deleted the File, the attempt to lock \c ptr will fail
//and the code in the if statement will not execute. Otherwise, it will
//work just like above.
if(boost::shared_ptr<File> h = ptr.lock())
{
	p->hi();
}

//If we just want to check if the object has been deleted, we can simply
//use the \c expired() function.
 if(ptr.expired())
	 cout<<"expired!";
