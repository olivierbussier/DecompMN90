#ifndef CDECOMPLIST_H
#define CDECOMPLIST_H


class cDecompList
{
    public:
        /** Default constructor */
        cDecompList();
        /** Default destructor */
        virtual ~cDecompList();
        /** Access m_Counter
         * \return The current value of m_Counter
         */
    protected:
    private:
        cDecompList *Next;
};

#endif // CDECOMPLIST_H
